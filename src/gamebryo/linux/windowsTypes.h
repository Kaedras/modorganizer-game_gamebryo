#pragma once

#include <cstdint>

using BYTE             = uint8_t;
using WORD             = uint16_t;
using DWORD            = uint32_t;
using ULONGLONG        = uint64_t;
using HKEY             = int;
using REFKNOWNFOLDERID = int;

enum keys
{
  HKEY_LOCAL_MACHINE,
  HKEY_CURRENT_USER
};
