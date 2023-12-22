#pragma once

#include <core/util/platform.h>
#include <core/util/marcos.h>

#if defined(__LINUX__)
#include <dlfcn.h>
#elif defined(__WIN__)
#include <Windows.h>
#endif
#include <string>
#include <unordered_map>


// TODO: Add a Library Manager
// TODO: Test me!
class Library
{
public:
  Library();
  ~Library();

  LY_NODISCARD static std::unique_ptr<Library> newLibrary(const char *dllName);

  bool load(const char* name);
  
  // void* getModule(const char *name);

  template <typename FnPtr>
  LY_NODISCARD FnPtr getModule(const char *name);
  void close();

  std::string lastError() const;
  std::string getDLLName() const { return dll_name_; }
private:
  std::string dll_name_;
#if defined(__LINUX__)
  void* module_{ nullptr };
#elif defined(__WIN__)
  HMODULE module_{ nullptr };
#endif
  bool hasModel_{false};
};
