/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#pragma once

#ifndef PVKWIZ_RESOURCE_H
#define PVKWIZ_RESOURCE_H

// Bitmaps
#define IDB_WATERMARK   0x01
#define IDB_HEADER      0x02

//  Strings
#define IDS_SELECTION_TITLE      0x01
#define IDS_IMPORT1_TITLE        0x02
#define IDS_IMPORT1_SUBTITLE     0x03
#define IDS_IMPORT2_TITLE        0x04
#define IDS_IMPORT2_SUBTITLE     0x05
#define IDS_EXPORT1_TITLE        0x06
#define IDS_EXPORT1_SUBTITLE     0x07
#define IDS_EXPORT2_TITLE        0x08
#define IDS_EXPORT2_SUBTITLE     0x09
#define IDS_CER_FILE_TYPE        0x0A
#define IDS_PVK_FILE_TYPE        0x0B
#define IDS_ISSUED_TO            0x0C
#define IDS_ISSUED_BY            0x0D
#define IDS_EXPIRATION_DATE      0x0E
#define IDS_SETTING_COLUMN       0x0F
#define IDS_VALUE_COLUMN         0x10
#define IDS_OPERATION            0x11
#define IDS_IMPORT               0x12
#define IDS_EXPORT               0x13
#define IDS_CER_FILE             0x14
#define IDS_PVK_FILE             0x15
#define IDS_PVK_PSWD             0x16
#define IDS_EXPORTABLE           0x17
#define IDS_USER_PROTECTED       0x18
#define IDS_CSP                  0x19
#define IDS_RSA_PROV             0x1A
#define IDS_DSS_PROV             0x1B
#define IDS_CERTIFICATE          0x1C
#define IDS_STRONG_ENC           0x1D
#define IDS_YES                  0x1E
#define IDS_NO                   0x1F
#define IDS_LINK                 0x20
#define IDS_CER_FILE_NOT_FOUND   0x21
#define IDS_PVK_FILE_NOT_FOUND   0x22
#define IDS_STORE_FAILED         0x23
#define IDS_BAD_KEY              0x24
#define IDS_PVK_NOT_FOUND        0x25
#define IDS_PVK_UNKNOWN_ERR      0x26
#define IDS_PVK_NOT_EXPORTABLE   0x27
#define IDS_INVALID_FILENAME     0x28
#define IDS_OVERWRITE            0x29
#define IDS_WARNING              0x2A
#define IDS_PSWD_DO_NOT_MATCH    0x2B
#define IDS_UNKNOWN_ERR          0x2C
#define IDS_IMPORTED             0x2D
#define IDS_IMPORTED_NO_PVK      0x2E
#define IDS_EXPORTED             0x2F

// Dialogs
#define IDD_START       0x0001
#define IDD_SELECTION   0x0002
#define IDD_IMPORT1     0x0003
#define IDD_IMPORT2     0x0004
#define IDD_EXPORT1     0x0005
#define IDD_EXPORT2     0x0006
#define IDD_SUMMARY     0x0007

// Controls
#define IDC_TITLE            0x0101
#define IDC_IMPORT           0x0102
#define IDC_EXPORT           0x0103
#define IDC_CER_FILE         0x0104
#define IDC_BROWSE_CER       0x0107
#define IDC_PVK_FILE         0x0105
#define IDC_BROWSE_PVK       0x0108
#define IDC_PVK_PSWD         0x0106
#define IDC_EXPORTABLE       0x0109
#define IDC_USER_PROTECTED   0x010A
#define IDC_RSA_PROV         0x010B
#define IDC_DSS_PROV         0x010C
#define IDC_CERTIFICATE      0x010D
#define IDC_SELECT_CERT      0x010E
#define IDC_VIEW_CERT        0x010F
#define IDC_CONFIRM_PSWD     0x0110
#define IDC_STRONG_ENC       0x0111
#define IDC_SUMMARY          0x0112
#define IDC_LINK             0x0113

#endif
