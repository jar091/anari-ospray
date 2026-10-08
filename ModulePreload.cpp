// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace anari_ospray {

// OSPRay opens its modules with a plain LoadLibrary, so Windows looks for
// their dependencies (Open VKL, Embree, TBB, ...) next to the application and
// in PATH, but not next to the module. That fails when the host application
// lives elsewhere and PATH does not list the OSPRay directory, e.g. when the
// device is loaded into a DCC tool. Load the module from the directory of this
// library up front, with its dependencies resolved from that same directory.
void preloadModule(const char *name)
{
#ifdef _WIN32
  HMODULE self = nullptr;
  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
              | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          (LPCWSTR)&preloadModule,
          &self))
    return;

  std::wstring path(MAX_PATH, L'\0');
  DWORD length = GetModuleFileNameW(self, path.data(), DWORD(path.size()));
  while (length == path.size()) {
    path.resize(path.size() * 2);
    length = GetModuleFileNameW(self, path.data(), DWORD(path.size()));
  }
  path.resize(length);
  const size_t separator = path.find_last_of(L"\\/");
  if (separator == std::wstring::npos)
    return;
  path.resize(separator + 1);

  path += L"ospray_module_";
  for (const char *c = name; *c; ++c)
    path += wchar_t(*c);
  path += L".dll";

  // Not finding it here is fine: ospLoadModule() then searches as usual.
  LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
  (void)name;
#endif
}

} // namespace anari_ospray
