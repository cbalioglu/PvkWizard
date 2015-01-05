/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "PvkWizard.h"

HFONT g_TitleFont, g_BoldFont;

INT_PTR
CALLBACK
BlgpStartPageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
    {
        HFONT Font = GetWindowFont(HwndDlg);
        if (Font)
        {
            LOGFONT LogFont;

            if (GetObject(Font, sizeof(LOGFONT), &LogFont) > 0)
            {
                HDC Hdc;

                LogFont.lfWeight = FW_BOLD;

                g_BoldFont = CreateFontIndirect(&LogFont);

                Hdc = GetDC(HwndDlg);

                LogFont.lfHeight = -MulDiv(GetDeviceCaps(Hdc, LOGPIXELSY), 12, 72);

                ReleaseDC(HwndDlg, Hdc);

                g_TitleFont = CreateFontIndirect(&LogFont);
            }
        }

        if (g_TitleFont)
        {
            SetWindowFont(GetDlgItem(HwndDlg, IDC_TITLE), g_TitleFont, TRUE);
        }

        if (g_ComCtrl6 && Font)
        {
            WCHAR Link[256];

            if (LoadString(g_Instance, IDS_LINK, Link, ARRAYSIZE(Link)) > 0)
            {
                RECT Rect = {195, 178, 130, 8};
                HWND Hwnd;

                MapDialogRect(HwndDlg, &Rect);

                Hwnd = CreateWindow(WC_LINK, Link, WS_VISIBLE | WS_CHILD,
                    Rect.left, Rect.top, Rect.right, Rect.bottom, HwndDlg, (HMENU) IDC_LINK, NULL, NULL);

                if (Hwnd)
                {
                    SetWindowFont(Hwnd, Font, FALSE);
                }
            }
        }

        break;
    }

    case WM_NOTIFY:
    {
        LPNMHDR Header = (LPNMHDR) LParam;

        if (Header->idFrom == IDC_LINK)
        {
            if (Header->code == NM_CLICK || Header->code == NM_RETURN)
            {
                ShellExecute(HwndDlg, L"open", ((NMLINK *) Header)->item.szUrl, NULL, NULL, SW_SHOWNORMAL);

                return TRUE;
            }

            break;
        }

        if (Header->code == PSN_SETACTIVE)
        {
            PropSheet_SetWizButtons(GetParent(HwndDlg), PSWIZB_NEXT);

            return TRUE;
        }

        break;
    }
    }

    return FALSE;
}