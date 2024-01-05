#include "H264Parser.h"
Nal H264Parser::findNal(const uint8_t* data, uint32_t size)
{
  if (size < 5) { return {}; }

  Nal nal{};

  uint32_t startCode = 0;
  uint32_t pos = 0;
  uint8_t prefix[3] = {0};
  memcpy(prefix, data, 3);
  size -= 3;
  data += 2;

  while(size--) {
    // 0x001
    if (prefix[pos % 3] == 0 && prefix[pos % 3 + 1] == 0 && prefix[pos % 3 + 2] == 1) {
      if (nal.start_pos == nullptr) {
        nal.start_pos = const_cast<uint8_t*>(data) + 1;
        startCode = 3;
      }
      else if (startCode == 3) {
        nal.end_pos = const_cast<uint8_t*>(data) - 3;
        break;
      }
    }
    // 0x0001
    else if (prefix[pos % 3] == 0 && prefix[pos % 3 + 1] == 0 && prefix[pos % 3 + 2] == 0) {
      if (*(data+1) == 0x01) {
        if (nal.start_pos == nullptr) {
          if (size >= 1) {
            nal.start_pos = const_cast<uint8_t *>(data) + 2;
          } else {
            break;
          }
          startCode = 4;
        }
        else if (startCode == 4) {
          nal.end_pos = const_cast<uint8_t *>(data) - 3;
          break;
        }
      }
    }

    prefix[(pos++) % 3] = *(++data);
  }

  return nal;
}
