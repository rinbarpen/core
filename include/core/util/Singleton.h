#pragma once

#include <mutex>

template <class T>
class Singleton
{
public:
  virtual ~Singleton() = default;
  static auto instance() -> T*
  {
    if (!pValue_) {
      mutex_.lock();
      if (!pValue_) {
        pValue_ = new T();
      }
      mutex_.unlock();
    }
    return pValue_;
  }
  
private:
  Singleton() = default;
  
  struct Deletor {
    ~Deletor()
    {
      if (pValue_)
        delete pValue_;
    }
  };

  Deletor deletor_;
  static inline std::mutex mutex_ = {};
  static inline T *pValue_ = nullptr;
};
