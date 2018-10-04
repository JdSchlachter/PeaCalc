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
#include <shellapi.h>
#include "ConfigHandler.h"
#include "Term.h"
#include "CommandHandler.h"

/** Compiler Settings: ****************************************************************/

#ifdef _MSC_VER
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"User32.lib"  )
#pragma comment(lib,"gdi32.lib"   )
#pragma warning(disable : 4100)
#pragma warning(disable : 4996)
#endif

/** Local Defines: ********************************************************************/

#define ID_EDIT       100
#define C_MINWIDTH    220
#define C_LINECOUNT   3

/** Global variables: *****************************************************************/

HWND            hWndMain;
HWND            hwndEdit;
WCHAR           szAppName[]     = TEXT("PeaCalc Portable");
WNDPROC         lpfnEditBoxLowProc;
DWORD           dwEditLastLF;
CConfigHandler  Config;
CCommandHandler Command(&Config);

/** Forward Declarations: *************************************************************/

LRESULT CALLBACK WndProc        (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditBoxProc    (HWND, UINT, WPARAM, LPARAM);
void             EditProcEnter  (HWND hwnd);
DWORD            dwFindNthLastCR(const WCHAR* pszwInput, int iCount);
void             vRollback      (WCHAR* pszwInput, WCHAR* pszwNewStart);

/** Application entry function: *******************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    /** Variables:                                                                    */
    MSG msg;
    WNDCLASS wndclass;
    /** Create and initialize configuration:                                          */
    if (!Config.bIsPortable()) szAppName[7] = 0;
    /** Prepare Window-Class:                                                         */
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    /** Try to register it:                                                           */
    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("This program requires at least Windows 2K!"), szAppName, MB_ICONERROR);
        return 0;
    }
    /** Create the window-handler:                                                    */
    hWndMain = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_TOOLWINDOW,
        szAppName,
        szAppName,
        WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX,
        Config.iLeft, Config.iTop, Config.iWidth, Config.iHeight,
        NULL, NULL, hInstance, NULL);
    /** And show it:                                                                  */
    ShowWindow(hWndMain, iCmdShow);
    UpdateWindow(hWndMain);
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}

/** Encapsulated creator of the edit-box: *********************************************/

HWND CreateEditBox(HWND hOwner, WPARAM wParam, LPARAM lParam) {
    /** Variables:                                                                    */
    HWND  hwndEdit;
    HFONT hFont;
    DWORD dwIndex;
    /** Create the edit-box-control:                                                  */
    hwndEdit = CreateWindow(TEXT("edit"), NULL,
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
        0, 0, 0, 0, hOwner, (HMENU)ID_EDIT,
        ((LPCREATESTRUCT)lParam)->hInstance, NULL);
    /** Overwrite its message-procedure and conserve the low-level one:               */
    lpfnEditBoxLowProc = (WNDPROC)SetWindowLongPtr(hwndEdit,GWLP_WNDPROC,(LONG_PTR)EditBoxProc );
    /** Set the font of the edit-box:                                                 */
    hFont = CreateFont(Config.iFontSize, 0, 0, 0,
        FW_DONTCARE,                  // nWeight
        FALSE,                        // bItalic
        FALSE,                        // bUnderline
        0,                            // cStrikeOut
        ANSI_CHARSET,                 // nCharSet
        OUT_DEFAULT_PRECIS,           // nOutPrecision
        CLIP_DEFAULT_PRECIS,          // nClipPrecision
        PROOF_QUALITY,                // nQuality
        VARIABLE_PITCH, TEXT("Consolas"));
    SendMessage(hwndEdit,             // Handle of edit control
        WM_SETFONT,                   // Message to change the font
        (WPARAM)hFont,                // handle of the font
        MAKELPARAM(TRUE, 0));
    /** Set the initial text:                                                         */
    SetWindowText(hwndEdit, Config.sText.c_str());
    /** Store the new starting-location:                                      */
    dwEditLastLF = dwFindNthLastCR(Config.sText.c_str(), 1);
    /** Set the selection at its end:                                                 */
    dwIndex = wcslen(Config.sText.c_str());
    SendMessage(hwndEdit, EM_SETSEL, dwIndex, dwIndex);
    /** And be done:                                                                  */
    return hwndEdit;
}

void CloseMain(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    /** Variables:                                                                    */
    RECT         rcWind;
    TCHAR        buffer[C_TEXTBUFSIZE];
    /** Get the windo-dimensions and store them:                                      */
    GetWindowRect(hwnd    , &rcWind);
    Config.iTop    = rcWind.top;
    Config.iLeft   = rcWind.left;
    Config.iHeight = (rcWind.bottom - rcWind.top);
    Config.iWidth  = (rcWind.right - rcWind.left);
    /** Get the edit-textz and store it:                                              */
    GetWindowText(hwndEdit, buffer, sizeof(buffer));
    Config.sText   = std::wstring(buffer);
    /** And send a quit message to the application:                                   */
    PostQuitMessage(0);
}

/** Main-Window Message-Handler: ******************************************************/

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    /** Variables:                                                                    */
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    RECT         rcClient, rcWind;
    /** Parse the window-message:                                                     */
    switch (message) {
    case WM_CREATE:
        /** Prepare window for opacity:                                               */
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        /** Create Edit-Box:                                                          */
        hwndEdit = CreateEditBox(hwnd, wParam, lParam);
        break;
    case WM_GETMINMAXINFO:
        /** Get the rectangles of window and client to calc the border-width:         */
        GetClientRect(hwnd, &rcClient);
        GetWindowRect(hwnd, &rcWind);
        /** And set the minimum size based as reply via lpMMI in lParam:              */
        lpMMI->ptMinTrackSize.x = C_MINWIDTH;
        lpMMI->ptMinTrackSize.y = ((rcWind.bottom - rcWind.top ) - rcClient.bottom) + (C_LINECOUNT * Config.iFontSize) + 2;
        break;
    case WM_SETFOCUS:
        /** When the window received the focus, pass it on to the edit-control:       */
        SetFocus(hwndEdit);
        return 0;
    case WM_SIZE:
        /** When the window is resized, do so with the edit-control:                  */
        MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
        return 0;
    case WM_COMMAND:
        /** If it is from the text-box, parse the text-box message:                   */
        if (LOWORD(wParam) == ID_EDIT) {
            switch (HIWORD(wParam)) {
            case EN_ERRSPACE:
            case EN_MAXTEXT:
                MessageBox(hwnd, TEXT("Edit control out of space."), szAppName, MB_OK | MB_ICONSTOP);
                return 0;
            case EN_SETFOCUS:
                /** When the text-box gets the focus, set the main-window strong:     */
                SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
                break;
            case EN_KILLFOCUS:
                /** When the text-box looses the focus, set the main-window opaque:   */
                SetLayeredWindowAttributes(hwnd, 0, (unsigned char) Config.iOpacity, LWA_ALPHA);
                break;
            }
        }
        break;
    case WM_DESTROY:
        /** Before the main-window is destroyed, store the window-properties:         */
        CloseMain(hwnd, wParam, lParam);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/** Edit-Box Message-Handler: *********************************************************/

LRESULT CALLBACK EditBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    DWORD      dwIndex;
    /** Do the message-processing:                                                    */
    switch ( message){
    case WM_KEYDOWN:
        /** Check, if it was a delete:                                                */
        if (wParam == VK_DELETE) {
            /** Check if it has to be ignored:                                        */
            SendMessage(hwnd, EM_GETSEL, (WPARAM)&dwIndex, NULL);
            if (dwIndex < (dwEditLastLF + 3)) return 0;
        }
        break;
    case WM_CHAR:
        /** Make sure, that CTRL+C is always passed on:                               */
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 3)) break;
        /** If it was a return, process it:                                           */
        if (wParam == VK_RETURN) {
            EditProcEnter(hwnd);
            return 0;
        }
        /** Fetch the input-position:                                                 */
        SendMessage(hwnd, EM_GETSEL, (WPARAM)&dwIndex, NULL);
        /** Check, if it is a BS to be ignored:                                       */
        if ((dwIndex < (dwEditLastLF + 4)) && (wParam == L'\b')) {
            return 0;
        }
        /** Check, if something was entered before the allowed start-position:        */
        if (dwIndex < (dwEditLastLF + 3)) {
            if (dwIndex > dwEditLastLF) {
                dwIndex = dwEditLastLF + 3;
            }else{
                dwIndex = GetWindowTextLength(hwnd);
            }
            /** Set the selection after the last character:                           */
            SendMessage(hwnd, EM_SETSEL, dwIndex, dwIndex);
        }
        /** Check, if it is a tilde '~', which will be replaced by a root-sign:       */
        if (wParam == 0x007E) {
            /** Mask out the keyboard-specifics:                                      */
            lParam = (lParam & 0xE000007FFF);
            /** And replace with the unicode:                                         */
            wParam = 0x221A;
        }
        /** Check, if it is a colon, which will be replaced by point:                 */
        if (wParam == 0x002C) {
            /** Mask out the keyboard-specifics:                                      */
            lParam = (lParam & 0xE000007FFF);
            /** And replace with the unicode:                                         */
            wParam = 0x002E;
        }
        /** Check, if it is a slash, which will be replaced by a division-sign:       */
        if (wParam == 0x002F) {
            /** Mask out the keyboard-specifics:                                      */
            lParam = (lParam & 0xE000007FFF);
            /** And replace with the unicode:                                         */
            wParam = 0x00F7;
        }
        break;
    }
    return CallWindowProc(lpfnEditBoxLowProc, hwnd, message, wParam, lParam);
}

void EditProcEnter(HWND hwnd) {
    TCHAR        buffer[C_TEXTBUFSIZE];
    DWORD        dwIndex;
    DWORD        dwStartPos;
    WCHAR*       pszwStart;
    /** Get the window-text:                                                  */
    GetWindowText(hwnd, buffer, sizeof(buffer));
    dwIndex = wcslen(buffer);
    /** Check, if there's enough to be processed:                             */
    if (dwIndex < (dwEditLastLF + 4)) return;
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
        SetWindowText(hwnd, buffer);
        SendMessage(hWndMain, WM_DESTROY, 0, 0);
        return;
    }else if (wcscmp(pszwStart, L"help") == 0) {
        /** Help - Externally open the manual:                                        */
        wcscat(buffer, L"> ");
        ShellExecute(NULL, L"open", L"PeaCalc.html", NULL, NULL , SW_SHOW );
    }else if (wcscmp(pszwStart, L"license") == 0) {
        /** License - Externally open the GPL:                                        */
        wcscat(buffer, L"> ");
        ShellExecute(NULL, L"open", L"LICENSE.txt", NULL, NULL , SW_SHOW );
    } else if (wcscmp(pszwStart, L"info") == 0) {
        wcscat(buffer, Config.pszwGetDefaultText());
    }else{
        /** Run the command-class and attach its output:                                  */
        wcscat(buffer, Command.sRun(std::wstring(pszwStart)).c_str());
    }
    /** Check, if the text has to be clipped:                                         */
    dwIndex = dwFindNthLastCR(buffer, Config.iLines);
    if (dwIndex>0) {
        vRollback (buffer, &buffer[dwIndex+1]);
    }
    SetWindowText(hwnd, buffer);
    /** Store the new starting-location:                                      */
    dwEditLastLF = dwFindNthLastCR(buffer, 1);
    /**  Set the selection after the last character:                          */
    dwIndex = wcslen(buffer);
    SendMessage(hwnd, EM_SETSEL, dwIndex, dwIndex);
    /** And trigger a scroll:                                                 */
    SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
}

DWORD dwFindNthLastCR(const WCHAR* pszwInput, int iCount) {
    DWORD dwPos = wcslen(pszwInput);
    while ((dwPos > 0) && (iCount>0)) {
        dwPos --;
        if (pszwInput[dwPos] == L'\n') iCount--;
    }
    return dwPos;
}

void vRollback(WCHAR* pszwInput, WCHAR* pszwNewStart) {
    while (*pszwNewStart != L'\0') {
        *pszwInput = *pszwNewStart;
        pszwInput ++;
        pszwNewStart ++;
    }
    *pszwInput = L'\0';
}
