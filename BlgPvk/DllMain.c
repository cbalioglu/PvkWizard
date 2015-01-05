/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>

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