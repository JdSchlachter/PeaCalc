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
#include <shellapi.h>
#include <string>
#include <algorithm>
#include <math.h>
#include "ConfigHandler.h"
#include "Term.h"
#include "CommandHandler.h"

/** Compiler Settings: ****************************************************************/

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

/** Public Functions: *****************************************************************/

/** Constructor: **********************************************************************/

CCommandHandler::CCommandHandler(CConfigHandler* Config) {
    m_pConfig = Config;
}

/** Destructor: ***********************************************************************/

CCommandHandler::~CCommandHandler() {
}

/** Tiny function to store a pointer to the info-text: ********************************/

void CCommandHandler::vSetInfoText(WCHAR* pszwTextPtr) {
    this->m_pszwInfoText = pszwTextPtr;
}

/** Set-function, which adds the info-text if there's empty input: ********************/

void CCommandHandler::vSetText(HWND hEditBox, const WCHAR* pszwNewText) {
    TCHAR  buffer[C_TEXTBUFSIZE];
    DWORD  dwIndex;
    if (pszwNewText[0] != L'\0') {
        wcscpy(buffer, pszwNewText);
    } else {
        wcscpy(buffer, m_pszwInfoText);
    }
    SetWindowText(hEditBox, buffer);
    /** Store the new starting-location:                                              */
    m_dwEditLastLF = dwFindNthLastCR(buffer, 1);
    /** Set the selection at its end:                                                 */
    dwIndex = wcslen(buffer);
    SendMessage(hEditBox, EM_SETSEL, dwIndex, dwIndex);
}

/** Handler for a command to be executed: *********************************************/

void CCommandHandler::vProcEnter(HWND hMain, HWND hEditBox) {
    TCHAR        buffer[C_TEXTBUFSIZE];
    DWORD        dwIndex;
    DWORD        dwStartPos;
    WCHAR*       pszwStart;
    /** Get the window-text:                                                          */
    GetWindowText(hEditBox, buffer, sizeof(buffer));
    dwIndex = wcslen(buffer);
    /** Check, if there's enough to be processed:                                     */
    if (dwIndex < (m_dwEditLastLF + 4)) return;
    /** Fetch the last CR.                                                            */
    /** Note, that this masks the end of the second to last line!                     */
    dwStartPos = dwFindNthLastCR(buffer, 1);
    /** When this is not the beginning of the string, move behind the CR:             */
    if (dwStartPos > 0) dwStartPos++;
    /** Check, if there's enough input:                                               */
    if (wcslen(buffer) < dwStartPos + 2) return;
    /** Cut out the last line as argument and the part before as history:             */
    pszwStart = &buffer[dwStartPos + 2];
    buffer[dwStartPos] = 0;
    /** Parse a command, if there is one:                                             */
    if (wcscmp(pszwStart, L"exit") == 0) {
        /** Exit - Send a destroy-message to the main window:                         */
        wcscat(buffer, L"> ");
        SetWindowText(hEditBox, buffer);
        SendMessage(hMain, WM_DESTROY, 0, 0);
        return;
    }
    else if (wcscmp(pszwStart, L"help") == 0) {
        /** Help - Externally open the manual:                                        */
        wcscat(buffer, L"> ");
        ShellExecute(NULL, L"open", L"PeaCalc.html", NULL, NULL, SW_SHOW);
    }
    else if (wcsncmp(pszwStart, L"license", 7) == 0) {
        /** License - Externally open the GPL:                                        */
        wcscat(buffer, L"> ");
        ShellExecute(NULL, L"open", L"LICENSE.txt", NULL, NULL, SW_SHOW);
    }
    else if (wcsncmp(pszwStart, L"clear", 5) == 0) {
        /** Clear - Clear the text-box text:                                          */
        wcscpy(buffer, L"> ");
    }
    else if (wcsncmp(pszwStart, L"info", 4) == 0) {
        /** Info - Add the info-text:                                                 */
        wcscat(buffer, L"  info\r\n");
        wcscat(buffer, m_pszwInfoText);
    }
    else if (wcsncmp(pszwStart, L"min", 3) == 0) {
        /** Min - Send a minimize-message to the main window:                         */
        wcscat(buffer, L"> ");
        SendMessage(hMain, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    }
    else {
        /** Run the mathematical handler and attach its output:                       */
        wcscat(buffer, vProcMath(std::wstring(pszwStart)).c_str());
    }
    /** Check, if the text has to be clipped:                                         */
    dwIndex = dwFindNthLastCR(buffer, m_pConfig->iLines);
    if (dwIndex>0) {
        vRollback(buffer, &buffer[dwIndex + 1]);
    }
    SetWindowText(hEditBox, buffer);
    /** Store the new starting-location:                                              */
    m_dwEditLastLF = dwFindNthLastCR(buffer, 1);
    /**  Set the selection after the last character:                                  */
    dwIndex = wcslen(buffer);
    SendMessage(hEditBox, EM_SETSEL, dwIndex, dwIndex);
    /** And trigger a scroll:                                                         */
    SendMessage(hEditBox, EM_SCROLLCARET, 0, 0);
}

/** Handler for a mathematical input: *************************************************/

std::wstring CCommandHandler::vProcMath(std::wstring sInput) {
    /** Variables:                                                                    */
    std::wstring sOutput;
    bool         bOutputHex = false;
    bool         bOutputBin = false;
    double       dOutput;
    INT32        s32Result;
    /** Save the input in the output-string:                                          */
    sOutput = L"  " + sInput + L"\r\n";
    /** Change the input to lower-case:                                               */
    std::transform(sInput.begin(), sInput.end(), sInput.begin(), ::tolower);
    /** Check for output-formatting:                                                  */
    if (sInput.substr(0,4) == L"hex(") {
        /** It shall be hexadecimal:                                                  */
        sInput = sInput.substr(3);
        bOutputHex = true;
    }else if (sInput.substr(0, 4) == L"bin(") {
        sInput = sInput.substr(3);
        bOutputBin = true;
    }
    /** Try to parse it:                                                              */
    s32Result = m_TermMain.s32Parse(sInput);
    if (s32Result == C_TERM_FuncOK      ) return (sOutput + L"  * Results in function!\r\n> ");
    if (s32Result != C_TERM_NumOK       ) return (sOutput + L"  * Parsing Error!\r\n> ");
    /** If we got here, the term can be calculated:                                   */
    s32Result = m_TermMain.s32Execute(0, &dOutput);
    if (s32Result == C_TERM_DivByZero   ) return (sOutput + L"  * Division by zero!\r\n> ");
    if (s32Result == C_TERM_BoolTooLarge) return (sOutput + L"  * Boolean operator too large!\r\n> ");
    /**                                                                               */
    /** Build up the output:                                                          */
    if (bOutputHex) {
        /** Build as hex:                                                             */
        if ((!isInteger(dOutput)) || (dOutput >= C_TERM_MAXINT)) {
            sOutput += L"  = " + sOutputHexFloat(dOutput) + L"\r\n> ";
        } else {
            sOutput += L"  = " + sOutputHexInt(dOutput) + L"\r\n> ";
        }
        return sOutput;
    }else if (bOutputBin) {
        /** Build as binary:                                                        */
        if (!isInteger(dOutput)) return (sOutput + L"  * Binary output only supported for integers!\r\n> ");
        if (dOutput >= C_TERM_MAXINT) return (sOutput + L"  * Result too large for binary output!\r\n> ");
        sOutput += L"  = " + sOutputBin(dOutput) + L"\r\n> ";
        return sOutput;
    }else if (isInteger(dOutput)) {
        /** Build as usual integer:                                                   */
        sOutput += L"  = " + sOutputInt(dOutput) + L"\r\n> ";
        return sOutput;
    }
    /** Build as some kind of float:                                                  */
    sOutput += L"  = " + sOutputFloat(dOutput) + L"\r\n> ";
    return sOutput;
}

/** Small support-function to scan for CRs: *******************************************/

DWORD CCommandHandler::dwFindNthLastCR(const WCHAR* pszwInput, int iCount) {
    DWORD dwPos = wcslen(pszwInput);
    while ((dwPos > 0) && (iCount>0)) {
        dwPos--;
        if (pszwInput[dwPos] == L'\n') iCount--;
    }
    return dwPos;
}

/** Formats an output as hex-int: *****************************************************/

std::wstring CCommandHandler::sOutputHexInt(double dInput) {
    WCHAR szwNumBuf[40];
    INT64 s64Temp = (INT64) dInput;
    swprintf(szwNumBuf, L"0x%I64X", s64Temp);
    return std::wstring(szwNumBuf);
}

/** Formats an output as hex-float: ***************************************************/

std::wstring CCommandHandler::sOutputHexFloat(double dInput) {
    tUnifNum num;
    WCHAR    szwNumBuf[40];
    num.f = (float) dInput;
    swprintf(szwNumBuf, L"0x%02X%02X%02X%02X f", num.u[3], num.u[2], num.u[1], num.u[0]);
    return std::wstring(szwNumBuf);
}

/** Formats an output as binary: ******************************************************/

std::wstring CCommandHandler::sOutputBin(double dInput) {
    /** Variables:                                                                    */
    WCHAR   szwNumBuf[80];
    INT64   s64Temp = (INT64)dInput;
    uint8_t u8Pos = 4;
    /** Find the right length:                                                        */
    while (((INT64)1 << u8Pos) <= abs(s64Temp)) u8Pos += 4;
    /** Build the according number of digits:                                         */
    wcscpy(szwNumBuf, L"0b");
    while (u8Pos>0) {
        /** Put spaces before each 4th digit:                                         */
        if (((u8Pos % 4) == 0) && (u8Pos>0)) wcscat(szwNumBuf, L" ");
        /** Go one bit further:                                                       */
        u8Pos--;
        /** And add the digit:                                                        */
        if (s64Temp & ((INT64)1 << u8Pos)) {
            wcscat(szwNumBuf, L"1");
        }else{
            wcscat(szwNumBuf, L"0");
        }
    }
    return std::wstring(szwNumBuf);
}

/** Formats an output as decimal integer: *********************************************/

std::wstring CCommandHandler::sOutputInt(double dInput) {
    WCHAR szwNumBuf[40];
    INT64 s64Temp = (INT64)dInput;
    swprintf(szwNumBuf, L"%I64i", s64Temp);
    return std::wstring(szwNumBuf);
}

/** Formats an output as floating-point value: ****************************************/

std::wstring CCommandHandler::sOutputFloat(double dInput) {
    WCHAR  szwNumBuf[40];
    WCHAR  szwFormat[40];
    double dTemp = dInput/10;
    int    iIntDigits = 1;
    int    iDecimals  = m_pConfig->iPrecision;
    /** Check, if the input is in the range for fixed-point:                          */
    if ((abs(dInput) < 1000000) && (abs(dInput) > 0.09)) {
        /** It is, so get the number of integer-digits:                               */
        iIntDigits = 1;
        while (abs(dTemp) > 1) {
            dTemp = dTemp / 10;
            iIntDigits++;
        }
        /** Make sure, that there's enough space for the precision:                   */
        if ((iIntDigits + iDecimals) > CNF_MAX_PRECISION) iDecimals = CNF_MAX_PRECISION - iIntDigits + 1;
        /** And build the output:                                                     */
        swprintf(szwFormat, L"%%1.%df", iDecimals);
        swprintf(szwNumBuf, szwFormat , dInput);
        return std::wstring(szwNumBuf);
    }
    /** It is not fixed-point, thus write it exponential style:                       */
    swprintf(szwFormat, L"%%1.%dE", m_pConfig->iPrecision);
    swprintf(szwNumBuf, szwFormat, dInput);
    return std::wstring(szwNumBuf);
}

/** Small support-functions: **********************************************************/

bool CCommandHandler::isInteger(double dInput) {
  double fractpart, intpart;
  fractpart = modf (dInput , &intpart);
  if (fractpart>0) return false;
  return true;
}

void CCommandHandler::vRollback(WCHAR* pszwInput, WCHAR* pszwNewStart) {
    while (*pszwNewStart != L'\0') {
        *pszwInput = *pszwNewStart;
        pszwInput++;
        pszwNewStart++;
    }
    *pszwInput = L'\0';
}

