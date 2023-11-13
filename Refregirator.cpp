// Refregirator.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>
#include <conio.h>
#include<Windows.h>
#include "strsafe.h"

#define ARROW_DOWN 80
#define ARROW_UP 72
#define CURSOR_LIMIT_UP 2
#define CURSOR_LIMINT_DOWN 5

/*
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD      dwSize;
    COORD      dwCursorPosition;
    WORD       wAttributes;
    SMALL_RECT srWindow;
    COORD      dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;
*/

typedef struct  {
    int x;
    int y;
} _CURSOR_POS;

//void ErrorExit(LPTSTR lpszFunction);
bool getCursorPosition(_CURSOR_POS* cursor_pos);
bool setCursorPosition(_CURSOR_POS* cursor_pos);
bool clearConsole();
void printMenu();

const char* MENU[] = {
    "1 >> Display Current Foods\n",
    "2 >> Remove Food\n",
    "3 >> New Food\n",
    "4 >> Exit\n"
};

_CURSOR_POS cursorPos = { 0.0 };

int main()
{

    cursorPos = { 0,2 };
    printMenu();

  
    char arrow;

    while (true) {
        switch (_getch()) {
        case 224 : 
            arrow = _getch();
            if (arrow == ARROW_DOWN) {
                if(cursorPos.y < CURSOR_LIMINT_DOWN) cursorPos.y++;
            }
            else if (arrow == ARROW_UP) {
                if (cursorPos.y > CURSOR_LIMIT_UP) cursorPos.y--;
            }
            break;
        default :
            break;
        }
        printMenu();
    }
}


bool getCursorPosition(_CURSOR_POS* cursor_pos) {
    CONSOLE_SCREEN_BUFFER_INFO presentCur;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur) == 0) {
        printf("Error !!! GetConsoleScreenBufferInfo");
        return false;
    }
    else {
        cursor_pos->x = presentCur.dwCursorPosition.X;
        cursor_pos->y = presentCur.dwCursorPosition.Y;
        printf("current cursor position  = %d, %d\n",cursor_pos->x, cursor_pos->y);
        return false;
    }
}

bool setCursorPosition(_CURSOR_POS* cursor_pos) {
    COORD newPosition = { (SHORT)cursor_pos->x, (SHORT)cursor_pos->y };
    if (SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), newPosition) == 0) {
        printf("Error in Setting Cursor to New Postion  = %d, %d\n", cursor_pos->x, cursor_pos->y);
        return false;
    }
    else {
        return true;
    }
}

bool clearConsole() {
    if (system("cls")) return true;
    else return false;
}

void setTextHighlight() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
}

void setTextNormal() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
}



void printMenu() {
    clearConsole();
     printf("커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n");
    printf("***********************************\n");
    int countOfMenu = sizeof(MENU) / sizeof(MENU[0]);
    for (int i = 0; i < countOfMenu; i++) {
        if (cursorPos.y - 2 == i) setTextHighlight();
        else setTextNormal();
        printf("%s", MENU[i]);
    }
    setTextNormal();
    printf("***********************************\n");
    setCursorPosition(&cursorPos);
}

/*
void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}
*/

