// Refregirator.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>
#include <conio.h>
#include<Windows.h>
#include "strsafe.h"
#include <time.h>

#define ARROW_DOWN 80
#define ARROW_UP 72
#define CURSOR_LIMIT_UP 4  // MENU 화면에서 CURSOR가 올라 갈 수 있는 한계
#define CURSOR_LIMINT_DOWN 8  // MENU 화면에서 CURSOR가 내려 갈 수 있는 한계
#define NO_OF_TITLE_LINE 4  // MENU화면에서 위의 빈줄 수

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

// 보관 물품 항목에 대한 구조체
typedef struct {
    char name[20];
    int count;
    time_t start_date;
    time_t expire_date;
} _ITEM;

// 커서 포지션에 대한 구조체
typedef struct  {
    int x;
    int y;
} _CURSOR_POS;


bool getCursorPosition(_CURSOR_POS* cursor_pos);
bool setCursorPosition(_CURSOR_POS* cursor_pos);
bool clearConsole();

// 시작하면서 _ITEM* 5개에 대한 memory 확보
bool initializedMemroyForReservedItem();

void printMenu();
void doAction(int option);
void doListItems();
// _ITEM 갯수가 5의 배수로 되면 남아있는 메모리 제거후 다시 메모리 획득기능 포함
bool doDeleteItem(); // remove 기본 작업
bool removeFromReservedItem(int itemNo); // 실제 remove 기능
// 새로운 아이템을 추가할 시 _ITEM 에 대한 메모리 모자라면 추가하는 기능 포함
bool doAddNewItem(); // add 기본 작업
bool addReservedItem(_ITEM* item); // 실제 add 기능
// 저장된 아이템을 수정하는 기능
bool doModifyItem(); // 수정 기본 작업
bool modifyReservedItem(int select); // 실제 수정 기능

// 위의 함수를 보조하기 위한  함수
bool printReservedItem();
char* getFormatedStringByTime_t(time_t* ttCurrent);


// 기본 화면에 표시할 메뉴 문자열
const char* MENU[] = {
    "1 >> Display Current Foods\n",
    "2 >> Remove Food\n",
    "3 >> New Food\n",
    "4 >> Modify Item\n",
    "5 >> Exit\n"
};

// 현재 cursor의 위치
_CURSOR_POS cursorPos = { 0.0 };

// 현재 냉장고 보관 물품 목록
_ITEM* reservedItem;

// 현재 냉장고 보관 품목 수
//reservedItem과 항상 동기를 정확히 시켜주어야 함
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

// 메뉴의 키가 선택되면 해당 함수를 호출
void doAction(int option) {
    //printf("Enter key was clicked in row %d", option);
    switch (option) {
    case NO_OF_TITLE_LINE : 
        doListItems();
        break;
    case NO_OF_TITLE_LINE+1:
        doDeleteItem();
        break;
    case NO_OF_TITLE_LINE+2:
        if (doAddNewItem() == false) { printf("Error !!! doAddNewItem()"); exit(1); };
        break;
    case NO_OF_TITLE_LINE+3:
        if(doModifyItem() == false ) { printf("Error !!! doModifyItem()"); exit(1); }
        break;
    case NO_OF_TITLE_LINE+4: 
        exit(0);
    default:break;
    }
}

// 전체 항목을 표시해 주는 역할
void doListItems() {
    clearConsole();
    printReservedItem();

    printf("확인 하셨으면 y 키를 누르세요 >>");
    while (_getch() != 'y') {};

}

// 새로운 항목을 추가 해 줌 - 실제 기능은 addReservedItem() 함수가 시행
bool doAddNewItem() {
    _ITEM newItem;
    clearConsole();
    printf("보관할 새로운 정보를 입력합니다 \n\n");
    // char name[20] '\0' 를 위해 19자만 입력 가능
    printf("품목 이름 : (19자이내) >> ");
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
    printf("유통기한 입력을 입력한다 (YYYY-MM-DD) >> ");
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
        printf("입력한 Expire date : %s\n", expDate);
        free(expDate);
    }

    if (addReservedItem(&newItem) == false) { printf("Error!!! fail to add ReservedItem()"); exit(0); }

    printf("\n확인 하셨으면 y 키를 누르세요 >>");
    while (_getch() != 'y') {};

    return true;
}

bool doDeleteItem() {
    clearConsole();
    printf("*****************************************\n");
    printf("삭제할 품목의 번호를 선택하고 엔터를 누르세요\n");
    printf("*****************************************\n");

    printReservedItem();

    int select;
    int countOfTry = 0;

    // buffer를 비움
    while (getchar() != '\n');

    while (true) {
        if (countOfTry == 3) break;

        printf("\n지울 아이템의  번호를 입력하세요 >> ");
        if (scanf_s("%d", &select) == 0) {
            countOfTry++;
            continue;
        }

        if (select < 0 || select >= countOfItems) {
            printf("잘못된 번호입니다\n");
            countOfTry++;
            continue;
        }

        break;
    }
    

    printf("\n 지울 항목 %d 맞으면 y,  아니라면  n 을 입력", select);
    while (true) {
        char answer = _getch();
        if (answer == 'y') {
            removeFromReservedItem(select);
            break;
        }
        else if (answer == 'n') break;
    } 
    
    return true;
}

bool doModifyItem() {
    clearConsole();
    printReservedItem();

    if (countOfItems == 0) {
        printf("\n 저장된 품목이 없습니다 되돌아가시려면 아무 키나 누르세요 >>");
        _getch();
        return true;
    }

    int select;
    int countOfTry = 0;

    // buffer를 비움
    while (getchar() != '\n');

    while (true) {
        if (countOfTry == 3) return false;
        printf("\n수정할 아이템의  번호를 입력하세요 >> ");
        if (scanf_s("%d", &select) == 0) {
            countOfTry++;
            continue;
        }
        if (select < 0 || select >= countOfItems) {
            printf("잘못된 번호입니다\n");
            countOfTry++;
            continue;
        }
        break;
    }

    printf("\n 수정할 항목이  [[ %d ]]  맞으면 y,  아니라면  n 을 입력 >>", select);
    while (true) {
        char answer = _getch();
        if (answer == 'y') {
            if( modifyReservedItem(select) == true) return true;
            else {
                printf("Error!!! modifyReservedItem  실행중 error 발생");
                return false;
            } 
        }
        else if (answer == 'n') break;
    }

    return true;
}


bool printReservedItem() {
    printf("\n------------------------------------------------------------------------------------------------\n");
    printf("%5s  %-20s  %-5s  %-30s  %-30s\n", "번호", "   품목 이름", "갯수", "입고날자", "유통기한");
    printf("------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < countOfItems; i++) {
        char* start_date = getFormatedStringByTime_t(&reservedItem[i].start_date);
        char* expire_date = getFormatedStringByTime_t(&reservedItem[i].expire_date);
        //printf("%-20s\n", expire_date);
        printf("%5d  %-20s  %3d개  %-30s  %-30s\n", i, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
        free(start_date);
        free(expire_date);
    }
    printf("------------------------------------------------------------------------------------------------\n");

    return true;
}

// time_t => YYYY-MM-DD 로 변환
// caller가 buff를 free 해 주어야 한다
char* getFormatedStringByTime_t(time_t* ttCurrent) {
    struct tm localTM;
    localtime_s(&localTM, ttCurrent);
    char* buff = (char*)malloc(sizeof(char)*256) ;
    if (buff == 0) { printf("Error getFormatedStringByTime_t\n"); exit(1); }
    else {
        asctime_s(buff, sizeof(char) * 256, &localTM);

        // 끝에 포함된 \n을 없앤다.
        size_t length = strlen(buff);
        if (length > 0 && buff[length - 1] == '\n') {
            buff[length - 1] = '\0'; 
        }

        return buff;
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
    printf("*********************************************************\n");
    printf("커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n\n");
    printf("*********************************************************\n");
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

// 실제적으로 reservedItem에서 해당 항목 제거,  메모리는  5의 배수 만큼 확보해 놓음
// delete no는 0부터 시작
bool removeFromReservedItem(int deleteNo) {
    if (deleteNo < 0 || deleteNo >= countOfItems) {
        printf("Error!!! removeFromReservedItem = 잘못된  index를 사용\n");
        return false;
    }

    // 새롭게 저장할 포인터
    _ITEM* tempItems;
    
    // 하나 뺐을 때의  필요 item  카운트수 를 구하고
    int requirdCount = ( (countOfItems-1)/5  + 1 ) * 5 ; // 원래 1-5  하나빼면 0-4 5개 확보,   원래 6-10 하나빼면 5-9 10개 확보
    
    // 메모리 확보
    tempItems = (_ITEM*)malloc( requirdCount * sizeof(_ITEM));
    if (tempItems == NULL) {
        printf("Error!!! removeFromReservedItem = 메모리 확보 실패\n");
        return false;
    }

    // for loop를 돌면서 지울 항목을 제외하고는 복사
    int savePosition = 0;
    for (int i = 0; i < countOfItems; i++) {
        // 지울 항목이면 pass
        if (i == deleteNo) continue;
        // 아니면  
        tempItems[savePosition] = reservedItem[i];
        savePosition++;
    }

    // 성공했으면 count  하나 빼줌
    countOfItems--;

    // 메모리를 해제하고 tempItems로 교체
    free(reservedItem);
    reservedItem = tempItems;

    return true;
}


bool modifyReservedItem(int select) {
    
    int retry = 0;
    char selectOption;

    while (true) {
        clearConsole();

        printf("\n------------------------------------------------------------------------------------------------\n");
        printf("% -20s % -5s % -30s % -30s\n", "   품목 이름", "갯수", "입고날자", "유통기한");
        printf("------------------------------------------------------------------------------------------------\n");

        char* start_date = getFormatedStringByTime_t(&reservedItem[select].start_date);
        char* expire_date = getFormatedStringByTime_t(&reservedItem[select].expire_date);
        printf("%-20s  %3d개  %-30s  %-30s\n", reservedItem[select].name, reservedItem[select].count, start_date, expire_date);
        free(start_date);
        free(expire_date);

        printf("------------------------------------------------------------------------------------------------\n");

        printf("\n수정할 항목에 따라 단축기를 누르세요 : 품목이름-p,   갯수-c,  입고날자-s, 유통기한-m, 종료-e >> ");
        if (retry == 3) {
            printf("\nWarning!!! 3번 이상 잘 못 입력하셨습니다\n");
            _getch();
            return true;
        }


        selectOption = _getch();
        switch (selectOption) {
        case 'p':
            char newName[20];
            printf("\n품목의 이름을   입력 하세요 19자 이하>> ");
            scanf_s("%19s", newName, (unsigned)_countof(newName));
            strcpy_s( reservedItem[select].name, newName);
            retry = 0;
            break;
        case 'c': 
            int newCount;
            printf("\n품목의 갯수를    입력 하세요 >> ");
            scanf_s("%d", &newCount);
            reservedItem->count = newCount;
            retry = 0;
            break;
        case 's': retry = 0; break;
            int newCount;
            printf("\n새로운 입고 날자를    입력 하세요 >> ");
            scanf_s("%d", &newCount);
            reservedItem->count = newCount;
            retry = 0;
            break;
        case 'm': retry = 0;  break;
        case 'e' :
            return true;
        default : 
            retry++;
            continue;
        }
    }

    return true;

}

// console로 부터 time_t구조를 입력 받아  return
time_t inputDateFromConsole(const char* message ) {
    // 기본 tm 구조 선언
    struct tm inputTime = { 0 };
    time_t retTime_t;

    //  data  입력
    printf("%s을(를) 다음 형식과 같이 입력하세요 (YYYY-MM-DD) >> ");
    int retry = 0;
    while (true) {
        if (retry == 3) {
            printf("\n 3회 이상 입력에 실패 했습니다, 아무키나 누르시면 되돌아 갑니다");
            _getch();
            return -1;
        }
        else if (scanf_s("%d-%d-%d", &inputTime.tm_year, &inputTime.tm_mon, &inputTime.tm_mday) != 3) {
            printf("정확한 날짜 정보를 입력하세요 e.g 2023-11-14 Error %d회\n", retry);
            retry++;
            while (getchar() != '\n'); // input buffer를 비운다
            continue;
        }
        else {
            inputTime.tm_year -= 1900;
            inputTime.tm_mon--;
            retTime_t = mktime(&inputTime);
            if (retTime_t == -1) {
                printf("입력하신 날자가 유효한 날자가 아닙니다.  다시 입력하세요\n");
                retry++;
                continue;
            } else {
                return retTime_t;
            }
        }
    }
}
