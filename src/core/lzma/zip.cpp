#include <core/lzma/zip.h>

#include <iostream>

LY_NAMESPACE_BEGIN

static auto g_lzma_logger = GET_LOGGER("lzma");
/*
bool compressFile(std::string_view inFile, std::string_view outFile) {
  std::ifstream fin(inFile.data(), std::ios::binary);
  if (!fin.is_open())
  {
    ILOG_ERROR(g_lzma_logger) << "Error opening input file!";
    return false;
  }

  std::ofstream fout(outFile.data(), std::ios::binary);
  if (!fout.is_open())
  {
    ILOG_ERROR(g_lzma_logger) << "Error opening output file!";
    return false;
  }

  z_stream defstream;
  defstream.zalloc = Z_NULL;
  defstream.zfree = Z_NULL;
  defstream.opaque = Z_NULL;
  defstream.avail_in = 0;
  defstream.next_in = Z_NULL;
  if (deflateInit(&defstream, Z_BEST_COMPRESSION) != Z_OK)
  {
    ILOG_ERROR(g_lzma_logger) << "deflateInit() failed!";
    return false;
  }

  uint8_t in[kChunkSize];
  uint8_t out[kChunkSize];
  int ret;
  do {
    fin.read(reinterpret_cast<char *>(in), kChunkSize);
    defstream.avail_in = static_cast<uint32_t>(fin.gcount());
    if (fin.bad())
    {
      (void) deflateEnd(&defstream);
      ILOG_ERROR(g_lzma_logger) << "Error reading from input file!";
      return false;
    }
    defstream.next_in = in;

    do {
      defstream.avail_out = kChunkSize;
      defstream.next_out = out;
      ret = deflate(&defstream, Z_FINISH);
      if (ret == Z_STREAM_ERROR)
      {
        (void) deflateEnd(&defstream);
        ILOG_ERROR(g_lzma_logger) << "deflate() failed!";
        return false;
      }
      fout.write(
        reinterpret_cast<const char *>(out), kChunkSize - defstream.avail_out);
    } while (defstream.avail_out == 0);
  } while (ret != Z_STREAM_END);

  (void) deflateEnd(&defstream);
  fin.close();
  fout.close();
  return true;
}

bool decompressFile(std::string_view inFile, std::string_view outFile)
{
  std::ifstream fin(inFile.data(), std::ios::binary);
  if (!fin.is_open())
  {
    ILOG_ERROR(g_lzma_logger) << "Error opening input file!";
    return false;
  }

  std::ofstream fout(outFile.data(), std::ios::binary);
  if (!fout.is_open())
  {
    ILOG_ERROR(g_lzma_logger) << "Error opening output file!";
    return false;
  }

  z_stream infstream;
  infstream.zalloc = Z_NULL;
  infstream.zfree = Z_NULL;
  infstream.opaque = Z_NULL;
  infstream.avail_in = 0;
  infstream.next_in = Z_NULL;
  if (inflateInit(&infstream) != Z_OK)
  {
    ILOG_ERROR(g_lzma_logger) << "inflateInit() failed!";
    return false;
  }

  uint8_t in[kChunkSize];
  uint8_t out[kChunkSize];
  int ret;
  do {
    fin.read(reinterpret_cast<char *>(in), kChunkSize);
    infstream.avail_in = static_cast<uint32_t>(fin.gcount());
    if (fin.bad())
    {
      (void) inflateEnd(&infstream);
      ILOG_ERROR(g_lzma_logger) << "Error reading from input file!";
      return false;
    }
    infstream.next_in = in;

    do {
      infstream.avail_out = kChunkSize;
      infstream.next_out = out;
      ret = inflate(&infstream, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR)
      {
        (void) inflateEnd(&infstream);
        ILOG_ERROR(g_lzma_logger) << "inflate() failed!";
        return false;
      }
      fout.write(
        reinterpret_cast<const char *>(out), kChunkSize - infstream.avail_out);
    } while (infstream.avail_out == 0);
  } while (ret != Z_STREAM_END);

  (void) inflateEnd(&infstream);
  fin.close();
  fout.close();
  return true;
}
*/
bool Zipper::compress(std::string_view inDir, std::string_view outZipFile)
{
  std::string command = "7z a -t";

  switch (zip_type_) {
  case _7Z:
    command += "7z";
  break;
  default:
    return false;
  }
  if (!password_.empty()) {
    command += " -p ";
    command += password_;
  }

  command += " \"";
  command += outZipFile;
  command += "\" ";
  command += " \"";
  command += inDir;
  command += "*\"";

  int r = ::system(command.c_str());
  return r == 0;
}

bool Zipper::decompress(std::string_view inZipFile, std::string_view outDirPath)
{
  std::string command = "7z x -t";

  switch (zip_type_)
  {
  case _7Z: command += "7z"; break;
  default: return false;
  }
  if (!password_.empty())
  {
    command += " -p ";
    command += password_;
  }

  command += " \"";
  command += inZipFile;
  command += "\" ";
  command += " -o\"";
  command += outDirPath;
  command += "\"";

  int r = ::system(command.c_str());
  return r == 0;
}

LY_NAMESPACE_END
