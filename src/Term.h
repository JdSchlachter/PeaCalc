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

#define C_TERM_NumOK             0x01
#define C_TERM_FuncOK            0x02
#define C_TERM_MissingBrk        0x03
#define C_TERM_MissingOperator   0x04
#define C_TERM_ErroneousNumeric  0x05
#define C_TERM_ParsingError      0x06
#define C_TERM_DivByZero         0x07

#define C_TERM_CmdEmpty          0x0000
#define C_TERM_CmdConstant       0x0001
#define C_TERM_CmdParameter      0x0002
#define C_TERM_CmdAddition       0x0003
#define C_TERM_CmdSubstraction   0x0004
#define C_TERM_CmdMultiplication 0x0005
#define C_TERM_CmdDivision       0x0006
#define C_TERM_CmdRoot           0x0007
#define C_TERM_CmdPower          0x0008
#define C_TERM_CmdLog            0x0009
#define C_TERM_CmdArcSin         0x000A
#define C_TERM_CmdArcCos         0x000B
#define C_TERM_CmdArcTan         0x000C
#define C_TERM_CmdSin            0x000D
#define C_TERM_CmdCos            0x000E
#define C_TERM_CmdTan            0x000F

/** Class Definition: *****************************************************************/

class CTerm {
public:
    CTerm();
    ~CTerm();
    void   vReset(void);
    INT32  s32Parse(const std::wstring sInput);
    INT32  s32Execute(const double dInput, double* pdOutput);
protected:
    bool   bRemoveSurroundingBrackets(std::wstring* psInput);
    INT32  s32ParseOperand(std::wstring sInput);
    INT32  s32OperatorFinder (const std::wstring sInput, const std::wstring sOperator);
    INT32  s32OperatorRevFind(const std::wstring sInput, const std::wstring sOperator);
    INT32  s32ParseOperator(const std::wstring sInput, UINT32* pu32OpType);
    INT32  s32Declare(std::wstring sInput);
private:
    CTerm* m_pSubT1;
    CTerm* m_pSubT2;
    double m_dVar;
    UINT32 m_u32Operator;
    
};
