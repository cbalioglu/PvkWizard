#include <windows.h>

#include "BlgPvk.h"
#include "BlgPvkp.h"

HANDLE g_Heap;

#ifndef BLGPVK_LIB_STATIC

BOOL
WINAPI
DllMain(
    IN HINSTANCE Instance,
    IN DWORD Reason,
    IN PVOID Reserved
    )
{
    UNREFERENCED_PARAMETER(Reserved);

    if(Reason == DLL_PROCESS_ATTACH)
    {
        g_Heap = GetProcessHeap();

        DisableThreadLibraryCalls(Instance);
    }

    return TRUE;
}

#endif