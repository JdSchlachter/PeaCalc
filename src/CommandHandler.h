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

/** Used Defines: *********************************************************************/

#pragma once

/** Class Definition: *****************************************************************/

class CCommandHandler {
public:
    DWORD           m_dwEditLastLF;
    CCommandHandler(CConfigHandler* Config);
    ~CCommandHandler();
    void            vSetInfoText(WCHAR* pszwTextPtr);
    void            vSetText(HWND hEditBox, const WCHAR* pszwNewText);
    void            vProcEnter(HWND hMain, HWND hEditBox);
    std::wstring    vProcMath(std::wstring sInput);
    DWORD           dwFindNthLastCR(const WCHAR* pszwInput, int iCount);
private:
    CTerm           m_TermMain;
    CConfigHandler* m_pConfig;
    WCHAR*          m_pszwInfoText;
    std::wstring    sOutputHexInt(double dInput);
    std::wstring    sOutputInt(double dInput);
    std::wstring    sOutputFloat(double dInput);
    bool            isInteger(double dInput);
    void            vRollback(WCHAR* pszwInput, WCHAR* pszwNewStart);

};
