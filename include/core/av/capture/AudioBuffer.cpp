#include "AudioBuffer.h"

AudioBuffer::AudioBuffer(size_t capacity) :
  capacity_(capacity)
{
  buffer_ = new uint8_t[capacity];
}
AudioBuffer::~AudioBuffer()
{
  delete[] buffer_;
}

int AudioBuffer::read(uint8_t* data, size_t len)
{
  Mutex::lock locker(mutex_);
  if (readableBytesNoLock() < len) {
    retrieve(0);
    return -1;
  }

  memcpy(data, buffer_ + read_pos_, len);
  retrieve(len);
  return len;
}
int AudioBuffer::write(const uint8_t* data, size_t len)
{
  Mutex::lock locker(mutex_);
  // if (writableBytesNoLock() < len) {
  //   uint8_t *new_data = new uint8_t[capacity_ * 2];
  //   memcpy(new_data, buffer_, capacity_);
  //   capacity_ = capacity_ * 2;
  //   delete[] buffer_;
  //   buffer_ = new_data;
  // }

  size_t writeBytes = writableBytesNoLock() < len ? writableBytesNoLock() : len;
  if (len > writableBytesNoLock()) len = writableBytesNoLock();

  if (len) {
    memcpy(buffer_ + write_pos_, data, len);
    write_pos_ += len;
  }
  
  return len;
}

void AudioBuffer::clear()
{
  Mutex::lock locker(mutex_);
  retrieveAll();
}

uint8_t* AudioBuffer::peek()
{
  Mutex::lock locker(mutex_);
  return peekNoLock();
}
const uint8_t* AudioBuffer::peek() const
{
  Mutex::lock locker(mutex_);
  return peekNoLock();
}

size_t AudioBuffer::readableBytes() const
{
  Mutex::lock locker(mutex_);
  return readableBytesNoLock();
}
size_t AudioBuffer::writableBytes() const
{
  Mutex::lock locker(mutex_);
  return writableBytesNoLock();
}
size_t AudioBuffer::size() const
{
  Mutex::lock locker(mutex_);
  return sizeNoLock();
}

void AudioBuffer::retrieve(size_t n)
{
  if (readableBytesNoLock() < n) {
    retrieveAll();
    return;
  }

  read_pos_ += n;
  if (read_pos_ == write_pos_) {
    read_pos_ = write_pos_ = 0;
  }
}
void AudioBuffer::retrieveAll()
{
  read_pos_ = write_pos_ = 0;
}
