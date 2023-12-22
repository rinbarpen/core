#pragma once

#include <iostream>
#include <string_view>
#include <core/util/marcos.h>
//#include <zlib/zlib.h>

#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN

inline constexpr int kChunkSize = 16384;

/**
 * @brief inFile and outFile does exist both!
 * @param inFile 
 * @param outFile 
 * @return 
 */
static bool compressFile(std::string_view inFile, std::string_view outFile);

/**
 * @brief inFile and outFile does exist both!
 * @param inFile 
 * @param outFile 
 * @return 
 */
static bool decompressFile(std::string_view inFile, std::string_view outFile);


class Zipper
{
public:
  enum ZipType
  {
    UNKNOWN,
    _7Z,
    _ZIP,
  };

  bool compress(std::string_view inDir, std::string_view outZipFile);
  bool decompress(std::string_view inZipFile, std::string_view outDirPath);

  void setZipType(ZipType type) { zip_type_ = type; }
  ZipType getZipType() const { return zip_type_; }
  
  void setMultiThreads() { use_multi_thread_  = true; }
  void unsetMultiThreads() { use_multi_thread_ = false; }

  void setPassword(const std::string &password) { password_ = password; }
  std::string getPassword() const { return password_; }

private:
  ZipType zip_type_{_7Z};
  std::string password_;
  LY_UNUSED bool use_multi_thread_{false};
};

LY_NAMESPACE_END
