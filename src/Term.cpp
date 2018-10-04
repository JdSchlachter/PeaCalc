//
//  This file is part of PeaCalc++ project
//  Copyright (C)2018 Jens Daniel Schlachter <gaius@slaughter.de>
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
#include <math.h>
#include "ConfigHandler.h"
#include "Term.h"

/** Public Functions: *****************************************************************/

CTerm::CTerm() {
    m_pSubT1 = NULL;
    m_pSubT2 = NULL;
}

CTerm::~CTerm() {
    vReset();
}

void CTerm::vReset(void) {
    if (m_pSubT1 != NULL) {
        delete m_pSubT1;
        m_pSubT1 = NULL;
    }
    if (m_pSubT2 != NULL) {
        delete m_pSubT2;
        m_pSubT2 = NULL;
    }
    m_u32Operator = C_TERM_CmdEmpty;
}

INT32 CTerm::s32Parse(std::wstring sInput) {
    vReset();
    return s32Declare(sInput);
}

INT32  CTerm::s32Execute(const double dInput, double* pdOutput) {
    INT32 iRes;
    double dPar1, dPar2;
    if (m_u32Operator == C_TERM_CmdParameter) {
        *pdOutput = dInput;
        return C_TERM_NumOK;
    }
    if (m_u32Operator == C_TERM_CmdConstant) {
        *pdOutput = m_dVar;
        return C_TERM_NumOK;
    }
    /** It is more than an operand, thus fetch the values: */
    iRes = m_pSubT1->s32Execute(dInput, &dPar1);
    if (iRes != C_TERM_NumOK) return iRes;
    iRes = m_pSubT2->s32Execute(dInput, &dPar2);
    if (iRes != C_TERM_NumOK) return iRes;
    /** And do the calculus: */
    switch (m_u32Operator) {
    case C_TERM_CmdAddition:
        *pdOutput = dPar1 + dPar2;
        return C_TERM_NumOK;
    case C_TERM_CmdSubstraction:
        *pdOutput = dPar1 - dPar2;
        return C_TERM_NumOK;
    case C_TERM_CmdMultiplication:
        *pdOutput = dPar1 * dPar2;
        return C_TERM_NumOK;
    case C_TERM_CmdDivision:
        if (dPar2 == 0) return C_TERM_DivByZero;
        *pdOutput = dPar1 / dPar2;
        return C_TERM_NumOK;
    case C_TERM_CmdPower:
        *pdOutput = pow(dPar1, dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdRoot:
        *pdOutput = pow(dPar2, 1 / dPar1);
        return C_TERM_NumOK;
    case C_TERM_CmdLog:
        *pdOutput = log(dPar2) / log(dPar1);
        return C_TERM_NumOK;
    case C_TERM_CmdArcSin:
        *pdOutput = asin(dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdArcCos:
        *pdOutput = acos(dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdArcTan:
        *pdOutput = atan(dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdSin:
        *pdOutput = sin(dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdCos:
        *pdOutput = cos(dPar2);
        return C_TERM_NumOK;
    case C_TERM_CmdTan:
        *pdOutput = tan(dPar2);
        return C_TERM_NumOK;
    }
    return C_TERM_NumOK;
};

/** Private Functions: ****************************************************************/

bool CTerm::bRemoveSurroundingBrackets(std::wstring* psInput) {
    /** Variables:                                                                    */
    int iLvl = 0;
    int iPos = 0;
    int iLen = psInput->length();
    /** Check, if there's actually something to do:                                   */
    if (psInput->front() != L'(') return false;
    /** Parse the string and count the bracket-levels:                                */
    while (iPos < (iLen-1)) {
        if (psInput->at(iPos) == L'(') iLvl++;
        if (psInput->at(iPos) == L')') iLvl--;
        /** If within the string the level reached zero, they are not surrounding:    */
        if (iLvl==0) return false;
        iPos++;
    }
    /** Check, if at the end there's a closure:                                       */
    if ((iLvl == 1) && (psInput->back() == L')')) {
        /** There is, so cut it:                                                      */
        *psInput = psInput->substr(1, iLen-2);
        return true;
    }
    /** If there wasnt, there's a bracket-error:                                      */
    return false;
}    

INT32 CTerm::s32ParseOperand(std::wstring sInput) {
    UINT64 u64Out;
    WCHAR* pwszEnd;
    int    iCount;
    if (sInput == L"x") {
        m_u32Operator = C_TERM_CmdParameter;
        return C_TERM_FuncOK;
    }
    if (sInput == L"e") {
        m_dVar = 2.718281828459045235360287471352;
        m_u32Operator = C_TERM_CmdConstant;
        return C_TERM_NumOK;
    }
    if (sInput == L"pi") {
        m_dVar = 3.141592653589793238462643383279;
        m_u32Operator = C_TERM_CmdConstant;
        return C_TERM_NumOK;
    }
    if (sInput.back() == L'°') {
        sInput = sInput.substr(0, sInput.length() - 1);
        m_dVar = wcstod(sInput.c_str(), &pwszEnd);
        if ((*pwszEnd) != L'\0') return C_TERM_ErroneousNumeric;
        m_dVar = m_dVar * 0.01745329251994329576923690768489;
        m_u32Operator = C_TERM_CmdConstant;
        return C_TERM_NumOK;
    }
    if (sInput.substr(0,2) == L"0b") {
        sInput = sInput.substr(2);
        iCount = 0;
        u64Out = 0;
        while (iCount < (int) sInput.length()) {
            u64Out = u64Out << 1;
            if (sInput[iCount] == L'1') {
                u64Out += 1;
            }else if (sInput[iCount] != L'0'){
                return C_TERM_ErroneousNumeric;
            }
            iCount++;
        }
        m_dVar = (double) u64Out;
        m_u32Operator = C_TERM_CmdConstant;
        return C_TERM_NumOK;
    }
    /** There's no special case, so try the standard:                       */
    m_dVar = wcstod(sInput.c_str(), &pwszEnd);
    if ((*pwszEnd) != L'\0') {
        return C_TERM_ErroneousNumeric;
    }
    m_u32Operator = C_TERM_CmdConstant;
    return C_TERM_NumOK;
};

INT32  CTerm::s32OperatorFinder(const std::wstring sInput, const std::wstring sOperator) {
    int iOpo  = -1;  // Operator Position
    int iBlvl = 0;   // Bracket Level
    /** NOTE: It is assumed, that surrounding brackets have been removed before!!     */
    while (iOpo < (int) sInput.length()) {
        iOpo++;
        if (sInput[iOpo] == L'(') iBlvl++;
        if (sInput[iOpo] == L')') iBlvl--;
        /** If at bracket-level-1, there's a bracket-error:                           */
        if (iBlvl == -1) return -1;
        /** If within brackets, continue for the next character:                      */
        if (iBlvl > 0) continue;
        /** If not within brackets, check for the operator:                           */
        if (sInput.substr(iOpo, sOperator.length()) == sOperator) {
            /** Directly take an operator at the beginning:                           */
            if (iOpo == 0) return iOpo;
			/** Ignore a minus, which is used for an exponent (thus 10 ^ ...):        */
            if ((sInput[iOpo-1] == L'e') &&
                (sOperator == L"-"     ) ){
                continue;
            }
            return iOpo;
        }
    }
    return -1;
}

INT32  CTerm::s32OperatorRevFind(const std::wstring sInput, const std::wstring sOperator) {
    int iOpo  = (int) sInput.length();  // Operator Position
    int iBlvl = 0;   // Bracket Level
    /** NOTE: It is assumed, that surrounding brackets have been removed before!!     */
    while (iOpo > 0) {
        iOpo--;
        if (sInput[iOpo] == L')') iBlvl++;
        if (sInput[iOpo] == L'(') iBlvl--;
        /** If at bracket-level-1, there's a bracket-error:                           */
        if (iBlvl == -1) return -1;
        /** If within brackets, continue for the next character:                      */
        if (iBlvl > 0) continue;
        /** If not within brackets, check for the operator:                           */
        if (sInput.substr(iOpo, sOperator.length()) == sOperator) {
            /** Directly take an operator at the beginning:                           */
            if (iOpo == 0) return iOpo;
			/** Ignore a minus, which is used for an exponent (thus 10 ^ ...):        */
            if ((sInput[iOpo-1] == L'e') &&
                (sOperator == L"-"     ) ){
                continue;
            }
            return iOpo;
        }
    }
    return -1;
}

INT32  CTerm::s32ParseOperator(const std::wstring sInput, UINT32* pu32OpType) {
    int iOpo;
    *pu32OpType = C_TERM_CmdEmpty;
    /** Try one by one the possible operators: */
    iOpo = s32OperatorFinder(sInput, L"+");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdAddition;
        return iOpo;
    }
    iOpo = s32OperatorRevFind(sInput, L"-");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdSubstraction;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"*");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdMultiplication;
        return iOpo;
    }
    iOpo = s32OperatorRevFind(sInput, L"÷");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdDivision;
        return iOpo;
    }
    iOpo = s32OperatorRevFind(sInput, L"/");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdDivision;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"√");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdRoot;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"^");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdPower;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"log(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdLog;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"asin(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdArcSin;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"acos(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdArcCos;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"atan(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdArcTan;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"sin(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdSin;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"cos(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdCos;
        return iOpo;
    }
    iOpo = s32OperatorFinder(sInput, L"tan(");
    if (iOpo != -1) {
        *pu32OpType = C_TERM_CmdTan;
        return iOpo;
    }
    return -1;
}

INT32  CTerm::s32Declare(std::wstring sInput) {
    INT32 iOpPos;
    INT32 iRes1, iRes2;
    /** Trim the string:                                                              */
    while (sInput.front() == L' ') sInput = sInput.substr(1);
    while (sInput.back () == L' ') sInput = sInput.substr(0, sInput.length()-1);
    while (bRemoveSurroundingBrackets(&sInput)) ;
    /**                                                                               */
    /** Lookup Operator:                                                              */
    iOpPos = s32ParseOperator(sInput, &m_u32Operator);
    /** Check, if by error the operator is at the end:                                */
    if (iOpPos == ((INT32) sInput.length() -1))  return C_TERM_ParsingError;
    /**                                                                               */
    /** If there is none, this is a pure argument:                                    */
    if (iOpPos == -1) return s32ParseOperand(sInput);
    /**                                                                               */
    /** Parse the first operand:                                                      */
    if (iOpPos == 0) {
        /** The should be an implicit operand:                                        */
        if (m_u32Operator == C_TERM_CmdLog) {
            m_pSubT1 = new CTerm();
            iRes1    = m_pSubT1->s32Parse(L"e");
        }
        else if (m_u32Operator == C_TERM_CmdRoot) {
            m_pSubT1 = new CTerm();
            iRes1 = m_pSubT1->s32Parse(L"2");
        }
        else if ((m_u32Operator == C_TERM_CmdArcSin) ||
            (m_u32Operator == C_TERM_CmdArcCos) ||
            (m_u32Operator == C_TERM_CmdArcTan) ||
            (m_u32Operator == C_TERM_CmdSin) ||
            (m_u32Operator == C_TERM_CmdCos) ||
            (m_u32Operator == C_TERM_CmdTan) ||
            (m_u32Operator == C_TERM_CmdSubstraction)) {
            /** There is no first operand expected:                                   */
            m_pSubT1 = new CTerm();
            iRes1 = m_pSubT1->s32Parse(L"0");
        }else{
            /** There should have been but is not:                                    */
            return C_TERM_ParsingError;
        }
    }else {
        if ((m_u32Operator == C_TERM_CmdArcSin) ||
            (m_u32Operator == C_TERM_CmdArcCos) ||
            (m_u32Operator == C_TERM_CmdArcTan) ||
            (m_u32Operator == C_TERM_CmdSin) ||
            (m_u32Operator == C_TERM_CmdCos) ||
            (m_u32Operator == C_TERM_CmdTan)) {
            /** There is no first operand expected:                                   */
            return C_TERM_ParsingError;
        }else {
            /** There is an explicit  defined first operand:                              */
            m_pSubT1 = new CTerm();
            iRes1 = m_pSubT1->s32Parse(sInput.substr(0, iOpPos));
        }
    }
    if ((iRes1 != C_TERM_NumOK) && (iRes1 != C_TERM_FuncOK)) return iRes1;
    /**                                                                               */
    /** Correct the operator-length:                                                  */
    if ((m_u32Operator == C_TERM_CmdArcSin) ||
        (m_u32Operator == C_TERM_CmdArcCos) ||
        (m_u32Operator == C_TERM_CmdArcTan)) {
        iOpPos += 3;
    }
    if ((m_u32Operator == C_TERM_CmdSin) ||
        (m_u32Operator == C_TERM_CmdCos) ||
        (m_u32Operator == C_TERM_CmdTan) ||
        (m_u32Operator == C_TERM_CmdLog)) {
        iOpPos += 2;
    }
    /**                                                                               */
    /** Parse the second operand:                                                     */
    m_pSubT2 = new CTerm();
    iRes2    = m_pSubT2->s32Parse(sInput.substr(iOpPos + 1));
    if ((iRes2 != C_TERM_NumOK) && (iRes2 != C_TERM_FuncOK)) return iRes2;
    /** Check the result and be gone:                                                 */
    if ((iRes1 == C_TERM_FuncOK) || (iRes2 == C_TERM_FuncOK)) return C_TERM_FuncOK;
    return C_TERM_NumOK;
}
