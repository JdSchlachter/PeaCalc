//
//  This file is part of PeaCalc++ project
//  Copyright (C)2018 Jens Daniel Schlachter <osw.schlachter@mailbox.org>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

/** Global Includes: ******************************************************************/

#include "stdafx.h"
#include <winuser.h>
#include <stdio.h>
#include <Shlobj.h>
#include <string>
#include "ConfigHandler.h"

/** Compiler Settings: ****************************************************************/

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

/** Local Constants: ******************************************************************/

const WCHAR cszwDefaultText[] = TEXT(" *  PeaCalc v1.0.2, Copyright (C) 2018 J.D. Schlachter\r\n *  This program comes with ABSOLUTELY NO WARRANTY.\r\n *  It is free software; you can redistribute it and/or modify it\r\n *  under the terms of the GNU General Public License version 3,\r\n *  or (at your option) any later version; type 'license' for details.\r\n *  Type 'info' for this notification.\r\n *  Type 'help' for the user-manual.\r\n> ");

/** Public Functions: *****************************************************************/

/** Constructor: **********************************************************************
 *    Triggers the check for portability and tries to load the settings               *
 *    from an ini-file either from the current path when running portable,            *
 *    or from the windows user's path:                                                */
 
CConfigHandler::CConfigHandler() {
    WCHAR szFileName[MAX_PATH];
    /** Check portability:                                                            */
    vCheckPortable();
    /** Setup the file-name:                                                          */
    if (bPortable) {
        /** If it is portable, it should be right there:                              */
        wcscpy(szFileName, L"PeaCalc.ini");
    }else{
        /** If it is not, try to build the path from the user-data-path:              */
        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, szFileName) != S_OK) {
            vSetDefaultData();
            return;
        }
        wcscat(szFileName, L"\\PeaCalc\\PeaCalc.ini");
    }
    if (!bReadFromFile(szFileName)) vSetDefaultData();
}

/** Destructor: ***********************************************************************
 *    Tries to store the settings in an ini-file either in the current path           *
 *    when running portable, or in the windows user's path:                           */

CConfigHandler::~CConfigHandler() {
    WCHAR szFileName[MAX_PATH];
    /** Setup the file-name:                                                          */
    if (bPortable) {
        /** If it is portable, it should be right there:                              */
        if (! bWriteToFile(L"PeaCalc.ini")) {
            MessageBox(NULL, TEXT("Failure in attempt to store\nconfiguration in portable ini-file!"), TEXT("PeaCalc Portable"), MB_OK | MB_ICONEXCLAMATION);
            return;
        }
    }else{
        /** If it is local, so fetch the user-directory:                              */
        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, szFileName) != S_OK) return;
        /** Create the directory in case it does not exist yet:                       */
        wcscat(szFileName, L"\\PeaCalc");
        if (!CreateDirectory(szFileName, NULL) && ERROR_ALREADY_EXISTS != GetLastError()) {
            /** It was not successful, and not because it already existed:            */
            MessageBox(NULL, TEXT("Failure in attempt to create\nsettings folder in user-directory!"), TEXT("PeaCalc Portable"), MB_OK | MB_ICONEXCLAMATION);
            return;
        }
        /** Add the actual file-name and try to save:                                 */
        wcscat(szFileName, L"\\PeaCalc.ini");
        if (! bWriteToFile(szFileName)) {
            MessageBox(NULL, TEXT("Failure in attempt to store\nconfiguration in user ini-file!"), TEXT("PeaCalc Portable"), MB_OK | MB_ICONEXCLAMATION);
        }
    }
}

/** Get-Function of the portability-flag: *********************************************
 *    This allows other classes to check, if the application runs                     *
 *    as portable:                                                                    */

bool CConfigHandler::bIsPortable(void) {
    return (bPortable);
}

/** Get-Function for the default-text: ************************************************
 *    This allows the main to fetch it for later display:                             */

const WCHAR* CConfigHandler::pszwGetDefaultText(void){
    return (cszwDefaultText);
}
/** Private Functions: ****************************************************************/

/** Check-function for portability: ***************************************************
 *    Sets the according flag to portable, when                                       *
 *     - an ini-file was found in the current directory or                            *
 *     - the drive, from which the application was started is a removable one.        */

void CConfigHandler::vCheckPortable(void) {
    /** Variables:                                                                    */
    WCHAR szFileName[MAX_PATH];
    DWORD dwDriveType;
    DWORD dwAttrib;
    HINSTANCE hInstance;
    /** Fetch the name and file-path, by which this application was started:          */
    hInstance  = GetModuleHandle(NULL);
    GetModuleFileName(hInstance, szFileName, MAX_PATH);
    /** Reduce it to the drive-letter and fetch the drive-type via the API:           */
    szFileName[2] = 0;
    dwDriveType = GetDriveType(szFileName);
    /** Try to fetch the file-attributes of the ini-file.                             *
      * Getting a valid result means, that the file exists.                           */
    dwAttrib = GetFileAttributes(TEXT("PeaCalc.ini"));
    /** When a removable drive or an existing ini-file was detected, then             *
      * this is running portable:                                                     */
    bPortable = (dwDriveType == DRIVE_REMOVABLE        ) ||
                (dwAttrib != INVALID_FILE_ATTRIBUTES &&
               !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}

/** File-Writer: **********************************************************************
 *    Writes the configuration into a plain text-file:                                */

bool CConfigHandler::bWriteToFile(const WCHAR* pszwFName) {
    /** Variables:                                                                    */
    FILE *fp;
    std::size_t i;
    /** Try to open the file:                                                         */ 
    fp = _wfopen(pszwFName, L"w, ccs=UTF-8");
    if (fp == NULL) return false;
    /** Write the items:                                                              */
    fwprintf(fp, L"[PeaCalc]\n");
    fwprintf(fp, L"Top=%d\n"      , iTop      );
    fwprintf(fp, L"Left=%d\n"     , iLeft     );
    fwprintf(fp, L"Height=%d\n"   , iHeight   );
    fwprintf(fp, L"Width=%d\n"    , iWidth    );
    fwprintf(fp, L"Opacity=%d\n"  , iOpacity  );
    fwprintf(fp, L"FontSize=%d\n" , iFontSize );
    fwprintf(fp, L"Precision=%d\n", iPrecision);
    fwprintf(fp, L"Lines=%d\n"    , iLines    );
    fwprintf(fp, L"[Text]\n"                  );
    /** Prepare the text bei removing \r:                                             */
    i = sText.find(L"\r");
    while (i != std::string::npos) {
        sText.erase(i, 1);
        i = sText.find(L"\r");
    }
    #ifdef _MSC_VER
        fwprintf(fp, L"%s"           , sText.c_str());
    #else
        fwprintf(fp, L"%S", sText.c_str());
    #endif
    /** And close:                                                                    */
    fclose(fp); 
    return true;
}

/** Entry-Parser: *********************************************************************
 *    Tries to read and parse a value with a given token from the ini-file:           */

INT32 CConfigHandler::iParseFileEntry(FILE *fp, const WCHAR* pszwToken, DWORD dwLim, INT32 ulDefault) {
    WCHAR buf[1000];
    DWORD dwRdVal;
    WCHAR *epr;
    /** Try to fetch a line from the file:                                            */
    if (fgetws(buf, sizeof(buf), fp) == NULL) return ulDefault;
    /** Check, if it contains the expected token:                                     */
    if (wcsncmp(buf, pszwToken, wcslen(pszwToken)) != 0) return ulDefault;
    /** Try to parse the numeric argument behind it:                                  */
    dwRdVal = wcstol(&buf[wcslen(pszwToken)], &epr, 10);
    /** When the result is not in bounds, it was not successful:                      */
    if ((dwRdVal < 1) || (dwRdVal > dwLim)) return ulDefault;
    /** If it was, the value can be written out:                                      */
    return (INT32)dwRdVal;
}

/** File-Reader: **********************************************************************
 *    Open the source-file and uses the parser above to fetch the configuration-      *
 *    values one by one:                                                              */

bool CConfigHandler::bReadFromFile(const WCHAR* pszwFName) {
    FILE *fp;
    WCHAR buf[1000];
    /** Try to open the file:                                                         */
    fp = _wfopen(pszwFName, L"r, ccs=UTF-8");
    if (fp == NULL) return false;
    /** Fetch the first line, and check if it is the expected header:                 */
    if (fgetws(buf, sizeof(buf), fp) == NULL) return false;
    if (wcsncmp(buf, L"[PeaCalc]", 9) != 0) return false;
    /** Use the parser to fetch and check the configuration-items:                    */
    iTop      = iParseFileEntry(fp, L"Top="      , CNF_MAX_TOP      , CNF_DEF_TOP      );
    iLeft     = iParseFileEntry(fp, L"Left="     , CNF_MAX_LEFT     , CNF_DEF_LEFT     );
    iHeight   = iParseFileEntry(fp, L"Height="   , CNF_MAX_HEIGHT   , CNF_DEF_HEIGHT   );
    iWidth    = iParseFileEntry(fp, L"Width="    , CNF_MAX_WIDTH    , CNF_DEF_WIDTH    );
    iOpacity  = iParseFileEntry(fp, L"Opacity="  , CNF_MAX_OPACITY  , CNF_DEF_OPACITY  );
    iFontSize = iParseFileEntry(fp, L"FontSize=" , CNF_MAX_FONTSIZE , CNF_DEF_FONTSIZE );
    iPrecision= iParseFileEntry(fp, L"Precision=", CNF_MAX_PRECISION, CNF_MAX_PRECISION);
    iLines    = iParseFileEntry(fp, L"Lines="    , CNF_MAX_LINES    , CNF_DEF_LINES    );
    if ((iLines & 1)==0) iLines++;
    /** Check the text-header:                                                        */
    if (fgetws(buf, sizeof(buf), fp) == NULL) return false;
    if (wcsncmp(buf, L"[Text]", 6) != 0) return false;
    /** And read the text line by line:                                               */
    sText = L"";
    while (fgetws(buf, sizeof(buf), fp) != NULL) {
        sText = sText + std::wstring(buf);
        /** Make sure to replace any pure\n by \r\n for windows:                      */
        if (sText.back() == L'\n') {
            sText.back() = L'\r';
            sText += L"\n";
        }
    }
    /** And close the file:                                                           */
    fclose(fp);
    return true;
}
/** Default-Configurator: *************************************************************
 *    This sets the configuration values to the default values, when anything         *
 *    failed while trying to read them from somewhere:                                */

void CConfigHandler::vSetDefaultData(void) {
    iTop       = CNF_DEF_TOP;
    iLeft      = CNF_DEF_LEFT;
    iHeight    = CNF_DEF_HEIGHT;
    iWidth     = CNF_DEF_WIDTH;
    iOpacity   = CNF_DEF_OPACITY;
    iPrecision = CNF_DEF_PRECISION;
    iFontSize  = CNF_DEF_FONTSIZE;
    iLines     = CNF_DEF_LINES;
    sText = std::wstring(cszwDefaultText);
}
