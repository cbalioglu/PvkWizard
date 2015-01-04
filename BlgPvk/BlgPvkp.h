#pragma once

#ifndef BLGPVKP_H
#define BLGPVKP_H

#include <windows.h>

#include "BlgPvk.h"

#define BLGPVK_FLAGON(x, Flag) (((x) & (Flag)) > 0)

extern HANDLE g_Heap;

BOOL
BLGPVKCALL
BlgpDeriveKey(
    IN HCRYPTPROV Provider,
    IN PBYTE Salt,
    IN DWORD SaltCb,
    IN PCSTR Password,
    IN DWORD Flags,
    OUT HCRYPTKEY *Key
    );

#endif