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

std::wstring CCommandHandler::sRun(std::wstring sInput) {
    /** Variables:                                                                    */
    std::wstring sOutput;
    bool         bOutputHex = false;
    double       dOutput;
    INT32        s32Result;
    /** Save the input in the output-string:                                          */
    sOutput = L"  " + sInput + L"\r\n";
    /** Change the input to lower-case:                                               */
    std::transform(sInput.begin(), sInput.end(), sInput.begin(), ::tolower);
    /** Check, if the output shall be as hex:                                         */
    if (sInput.substr(0,4) == L"hex(") {
        sInput = sInput.substr(3);
        bOutputHex = true;
    }
    /** Try to parse it:                                                              */
    s32Result = m_TermMain.s32Parse(sInput);
    if (s32Result == C_TERM_FuncOK   ) return (sOutput + L"  * Results in function!\r\n> ");
    if (s32Result != C_TERM_NumOK    ) return (sOutput + L"  * Parsing Error!\r\n> ");
    /** If we got here, the term can be calculated:                                   */
    s32Result = m_TermMain.s32Execute(0, &dOutput);
    if (s32Result == C_TERM_DivByZero) return (sOutput + L"  * Division by zero!\r\n> ");
    /**                                                                               */
    /** Build up the output:                                                          */
    if (bOutputHex) {
        /** Build as hex:                                                             */
        /** Note, this does not support IEEE 754 yet!                                 */
        if (!isInteger(dOutput)) return (sOutput + L"  * No support (yet) of float-output in IEEE754!\r\n> ");
        /** Limt at 2^53, since the output comes double:                              */
        if (dOutput > 9007199254740991) return (sOutput + L"  * Result too large to convert to hex!\r\n> ");
        sOutput += L"  = " + sOutputHexInt(dOutput) + L"\r\n> ";
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

bool CCommandHandler::isInteger(double dInput) {
  double fractpart, intpart;
  fractpart = modf (dInput , &intpart);
  if (fractpart>0) return false;
  return true;

}

std::wstring CCommandHandler::sOutputHexInt(double dInput) {
    WCHAR szwNumBuf[40];
    INT64 s64Temp = (INT64) dInput;
    swprintf(szwNumBuf, L"0x%I64X", s64Temp);
    return std::wstring(szwNumBuf);
}

std::wstring CCommandHandler::sOutputInt(double dInput) {
    WCHAR szwNumBuf[40];
    INT64 s64Temp = (INT64)dInput;
    swprintf(szwNumBuf, L"%I64i", s64Temp);
    return std::wstring(szwNumBuf);
}

std::wstring CCommandHandler::sOutputFloat(double dInput) {
    WCHAR  szwNumBuf[40];
    WCHAR  szwFormat[40];
    double dTemp = dInput/10;
    int    iIntDigits = 1;
    int    iDecimals  = m_pConfig->iPrecision;
    /** Check, if the input is in the range for fixed-point:                          */
    if ((abs(dInput) < 100000) && (abs(dInput) > 0.1)) {
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