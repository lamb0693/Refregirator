// Refregirator.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>
#include <conio.h>
#include<Windows.h>
#include "strsafe.h"
#include <time.h>

#define ARROW_DOWN 80
#define ARROW_UP 72
#define CURSOR_LIMIT_UP 3  // CURSOR가 올라 갈 수 있는 한계
#define CURSOR_LIMINT_DOWN 6  // CURSOR가 내려 갈 수 있는 한계
#define NO_OF_TITLE_LINE 3  // 위의 빈줄 수

/*
*  CONSOLE_SCRREN_BUFFER_INFO 구조
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD      dwSize;
    COORD      dwCursorPosition;
    WORD       wAttributes;
    SMALL_RECT srWindow;
    COORD      dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;
*/

typedef struct {
    char name[20];
    int count;
    time_t start_date;
    time_t expire_date;
} _ITEM;

typedef struct  {
    int x;
    int y;
} _CURSOR_POS;

//void ErrorExit(LPTSTR lpszFunction);
bool getCursorPosition(_CURSOR_POS* cursor_pos);
bool setCursorPosition(_CURSOR_POS* cursor_pos);
bool clearConsole();
void printMenu();
bool initializedMemroyForReservedItem();
bool addReservedItem(_ITEM* item);
void doAction(int option);
bool doAddNewItem();
char* getFormatedStringByTime_t(time_t* ttCurrent);

const char* MENU[] = {
    "1 >> Display Current Foods\n",
    "2 >> Remove Food\n",
    "3 >> New Food\n",
    "4 >> Exit\n"
};

_CURSOR_POS cursorPos = { 0.0 };

_ITEM* reservedItem;
int countOfItems = 0;



int main()
{
    initializedMemroyForReservedItem();


    cursorPos = { 0, NO_OF_TITLE_LINE };
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

        case '\r' :
            doAction(cursorPos.y);
        default :
            break;
        }
        printMenu();
    }
}

void doAction(int option) {
    printf("Enter key was clicked in row %d", option);
    switch (option) {
    case NO_OF_TITLE_LINE : break;
    case NO_OF_TITLE_LINE+1: break;
    case NO_OF_TITLE_LINE+2:
        if (doAddNewItem() == false) { printf("Error !!! doAddNewItem()"); exit(1); };
        break;
    case NO_OF_TITLE_LINE+3: 
        exit(1);
    default:break;
    }
}

bool doListItems() {
    return true;
}

bool doAddNewItem() {
    _ITEM newItem;
    clearConsole();
    printf("보관할 새로운 정보를 입력합니다 \n\n");
    printf("품목 이름 : (20자이내) >> ");
    scanf_s("%s", newItem.name, (unsigned)_countof(newItem.name));
    printf("품목 갯수 >> ");
    scanf_s("%d", &newItem.count );
    
    // start_time에 현재 날자 배정
    time(&newItem.start_date);
    char* startDate = getFormatedStringByTime_t(&newItem.start_date);
    printf("입고 날자 : %s", startDate);
    free(startDate);

    // 유통기한 입력하기
    struct tm inputTime = { 0 };
    printf("Enter date and time (YYYY-MM-DD) >> ");
    int retry = 0;
    while(scanf_s("%d-%d-%d",  &inputTime.tm_year, &inputTime.tm_mon, &inputTime.tm_mday) != 3) {
        retry++;
        if (retry == 3) return false;
        printf("정확한 날자 정보를 입력하세요 e.g 2023-11-14 Error %d회\n", retry); 
    }

    // 입력한 날자 형식이 문제가 없는지 확인 없으면 출력한다
    inputTime.tm_year -= 1900; 
    inputTime.tm_mon--;        
    newItem.expire_date = mktime(&inputTime);
    if (newItem.expire_date == -1) {
        printf("Invalid date and time.  \n");
        return false;
    }
    else {
        char* expDate = getFormatedStringByTime_t(&newItem.expire_date);
        printf("입력한 Expire date : %s", expDate);
        free(expDate);
    }

    if (addReservedItem(&newItem) == false) { printf("Error!!! fail to add ReservedItem()"); exit(0); }

    printf("확인 하셨으면 y 키를 누르세요 >>");
    while (_getch() != 'y') {};
}

// time_t => YYYY-MM-DD 로 변환
// caller가 buff를 free 해 주어야 한다
char* getFormatedStringByTime_t(time_t* ttCurrent) {
    struct tm localTM;
    localtime_s(&localTM, ttCurrent);
    char* buff = (char*)malloc(sizeof(char)*256) ;
    asctime_s(buff, sizeof(char)*256 , &localTM);
    return buff;
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
    printf("커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n\n");
    printf("***********************************\n");
    int countOfMenu = sizeof(MENU) / sizeof(MENU[0]);
    for (int i = 0; i < countOfMenu; i++) {
        if (cursorPos.y - NO_OF_TITLE_LINE == i) setTextHighlight();
        else setTextNormal();
        printf("%s", MENU[i]);
    }
    setTextNormal();
    printf("***********************************\n");
    setCursorPosition(&cursorPos);
}

// 일단 처음에 5개 영역 확보
bool initializedMemroyForReservedItem() {

    reservedItem = (_ITEM*) malloc(5 * sizeof(_ITEM));
    if (reservedItem == NULL) {
        printf("Error!!! Cannot allocate memory for reservedItem ");
        return false;
    }
    else {
        return true;
    }
}

// 메모리 reallocation은 5의 변수 일때만
// 메모리가 남아있으면 item add
// 메모리가 모자라면 새로 allocation 후 대입
// copy를 하믈 *item은 메모리를 유지할 필요가 없다.
bool addReservedItem(_ITEM* item) {
    // 메모리가 모자라면 새로운 memory를 확보 후 이전 데이터를 복사하고 현재 내용을 덧 붙임
    _ITEM* tempItems;

    if (countOfItems % 5 == 0) {
        tempItems = (_ITEM*)malloc((countOfItems + 5) * sizeof(_ITEM));
        if (tempItems == 0) {
            printf("Error!!! cannot allocate for memory to tempItems");
            return false;
        }
        else {
            memcpy(tempItems, reservedItem, countOfItems * sizeof(_ITEM));
            free(reservedItem);
            reservedItem = tempItems;
        }
    }

    reservedItem[countOfItems] = *item;
    countOfItems++;
    return true;
}


