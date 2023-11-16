// Refregirator.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>
#include <conio.h>
#include<Windows.h>
#include "strsafe.h"
#include <time.h>

#define ARROW_DOWN 80
#define ARROW_UP 72
#define COUNT_OF_MAIN_MENU 7

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

// main menu의 cursor 위치를 저장할 변수
_CURSOR_POS menuPosition[COUNT_OF_MAIN_MENU];

// main menu의 enum 상수
enum MENU_ITEM {
    CURRENT_STATUS, REMOVE_FOOD, NEW_FOOD, MODIFY_ITEM, READ_FROM_FILE, WRITE_TO_FILE, EXIT
};
// 현재  select 된 main menu
enum MENU_ITEM selectedMenu = CURRENT_STATUS;

// 냉장고 상태보기 내에서 사용되는 enum 상수
enum STATUS_BUTTON_MENU {
    RECIPE, REF_TEMP_UP, REF_TEMP_DOWN, FRE_TEMP_UP, FRE_TEMP_DOWN, PREVIOUS
};



// data를 저장할 파일 이름
const char* strFileName = "refregirator.rfr";

// console의 cursor 관련 함수
bool getCursorPosition(_CURSOR_POS* cursor_pos);
bool setCursorPosition(_CURSOR_POS* cursor_pos);
bool clearConsole();

// 시작하면서 _ITEM* 5개에 대한 memory 확보
bool initializedMemroyForReservedItem();

// 기본 메뉴 표시 화면
void printMenu();

// option에 따른  함수의 분기 지정
void doAction(int option);

// 현재 _ITEM* 보여주기
void doListItems();
void printRefregiratorStatus(_CURSOR_POS buttonPos[6], enum STATUS_BUTTON_MENU currentSelection);

// _ITEM 항목 삭제 
// // _ITEM 갯수가 5의 배수로 되면 남아있는 메모리 제거후 다시 메모리 획득기능 포함
bool doDeleteItem(); // remove 기본 작업
bool removeFromReservedItem(int itemNo); // 실제 remove 기능

// 새로운 아이템 추가
// 새로운 아이템을 추가할 시 _ITEM 에 대한 메모리 모자라면 추가하는 기능 포함
bool doAddNewItem(); // add 기본 작업
bool addReservedItem(_ITEM* item); // 실제 add 기능

// 저장된 아이템을 수정하는 기능
bool doModifyItem(); // 수정 기본 작업
bool modifyReservedItem(int select); // 실제 수정 기능

// 저장된 file에서 불러 오기 및 쓰기
bool doReadFromFile();
bool doSaveToFile();

// 위의 함수를 보조하기 위한  함수
bool printReservedItem();  // 현재 지정된 항목을 list하기 위한 함수
char* getFormatedStringByTime_t(time_t* ttCurrent);  // time_t에서  날짜 스트링으로 변환 실행측에서 memory해제해야 함
time_t inputDateFromConsole(const char* message);  // time_t 구조를 입력 받기 위한 함수
void setTextHighlight();
void setTextNormal();


// 기본 화면에 표시할 메뉴 문자열
const char* MENU[] = {
    "0 >> 현재 상황 및 음식 출력\n",
    "1 >> Remove Food\n",
    "2 >> New Food\n",
    "3 >> Modify Item\n",
    "4 >> File에서 불러오기\n",
    "5 >> File에 현상태 저장하기\n",
    "6 >> Exit\n"
};



// 현재 cursor의 위치
//_CURSOR_POS cursorPos = { 0.0 };

// 현재 냉장고 보관 물품 목록
_ITEM* reservedItem;

// 현재 냉장고 보관 품목 수
//reservedItem과 항상 동기를 정확히 시켜주어야 함
int countOfItems = 0;

// 냉장고 온도
int refregiratorTemperature = 5;
int freezerTemperature = -5;


int main()
{
    initializedMemroyForReservedItem();

    printMenu();

    char arrow; // arrow를 누르면 224, ARROW_DOWN, ARROW_UP 값이 순차적으로 들어 옴

    while (true) {
        switch (_getch()) {
        case 224 : //특수 키면 ArrowUp인지 ArrowDown인지 확인
            arrow = _getch();
            if (arrow == ARROW_DOWN) {
                if(selectedMenu < COUNT_OF_MAIN_MENU-1) selectedMenu = (enum MENU_ITEM)(selectedMenu + 1);
            }
            else if (arrow == ARROW_UP) {
                if (selectedMenu >0 ) selectedMenu = (enum MENU_ITEM)(selectedMenu - 1);
            }
            break;

        case '\r' :  // Enter key가 눌려지면 실제 행동을 한다
            doAction(selectedMenu);
        default :
            break;
        }
        printMenu();
    }
}

void printMenu() {
    clearConsole();
    printf("*********************************************************\n");
    printf("커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n\n");
    printf("*********************************************************\n");
    int countOfMenu = sizeof(MENU) / sizeof(MENU[0]);
    for (int i = 0; i < countOfMenu; i++) {
        if (selectedMenu == i) setTextHighlight();
        printf("%s", MENU[i]);
        setTextNormal();
    }
    printf("***********************************\n");
}

// 메뉴의 키가 선택되면 해당 함수를 호출
void doAction(int option) {

    switch (option) {
    case CURRENT_STATUS:   //첫째 줄
        doListItems();
        break;
    case REMOVE_FOOD:  // 둘째 줄
        doDeleteItem();
        break;
    case NEW_FOOD:
        if (doAddNewItem() == false) { printf("Error !!! doAddNewItem()"); exit(1); };
        break;
    case MODIFY_ITEM:
        if(doModifyItem() == false ) { printf("Error !!! doModifyItem()"); exit(1); }
        break;
    case READ_FROM_FILE:
        if (doReadFromFile() == false) { printf("Error !!! doReadFromFile()"); exit(1); }
        break;
    case WRITE_TO_FILE:
        if (doSaveToFile() == false) { printf("Error !!! doSaveToFile()"); exit(1); }
        break;
    case EXIT:
        exit(0);
    default:break;
    }
}

// 전체 항목을 표시해 주는 역할
void doListItems() {
    // 가상 버튼 6개
    _CURSOR_POS buttonPos[6];

    // 현재 선택된 메뉴 설정
    enum STATUS_BUTTON_MENU currentSelection = PREVIOUS;
 
    //  화살표 키 입력을 받을 변수
    int arrow;

    // 현상태를 인쇄
    printRefregiratorStatus(buttonPos, currentSelection);

    bool completed = false;
    while (!completed) {
        switch (_getch()) {
        case 224: //특수 키면 ArrowUp인지 ArrowDown인지 확인
            arrow = _getch();
            if (arrow == ARROW_DOWN) {
                if (currentSelection <5) currentSelection = (enum STATUS_BUTTON_MENU)(currentSelection+1);
            }
            else if (arrow == ARROW_UP) {
                if (currentSelection > 0) currentSelection = (enum STATUS_BUTTON_MENU)(currentSelection - 1);
            }
            break;

        case '\r':  // Enter key가 눌려지면 실제 행동을 한다
            switch (currentSelection) {
            case RECIPE:
                break;
            case REF_TEMP_UP:
                refregiratorTemperature++;
                break;
            case REF_TEMP_DOWN:
                refregiratorTemperature--;
                break;
            case FRE_TEMP_UP:
                freezerTemperature++;
                break;
            case FRE_TEMP_DOWN:
                freezerTemperature--;
                break;
            case PREVIOUS:
                completed = true;
                break;
            default:
                break;
            }
        }
        printRefregiratorStatus(buttonPos, currentSelection);
    }

}

void printRefregiratorStatus(_CURSOR_POS buttonPos[6] ,enum STATUS_BUTTON_MENU currentSelection) {
    clearConsole();

    // 현재 시간
    time_t currentTime_t;
    time(&currentTime_t);

    printf("-------------------------------------------------------------------");
    printf("\n냉장고 관리 시스템");
    char* strCurrentTime = getFormatedStringByTime_t(&currentTime_t);
    printf("\n%s", strCurrentTime);
    free(strCurrentTime);

    printf("\n-------------------------------------------------------------------");
    printf("\n유통기한 임박");
    for (int i = 0; i < countOfItems; i++) {
        if (reservedItem[i].expire_date < currentTime_t) printf("\n%20s 가 유통기한이 경과하였습니다", reservedItem[i].name);
        else if (difftime(reservedItem[i].expire_date, currentTime_t) < 60 * 60 * 24) printf("\n%20s가 유통기한이 하루 이하입니다", reservedItem[i].name);
    }

    printf("\n-------------------------------------------------------------------");
    printf("\n지금 만들 수 있는 메뉴\n");
    getCursorPosition(&buttonPos[RECIPE]);
    if (currentSelection == RECIPE) setTextHighlight();
    printf("[레시피 관리]");
    setTextNormal();

    printf("\n-------------------------------------------------------------------");
    printf("\n냉장실(%3d도)", refregiratorTemperature);
    
    getCursorPosition(&buttonPos[REF_TEMP_UP]);
    if (currentSelection == REF_TEMP_UP) setTextHighlight();
    printf(" [온도올리기] ");
    setTextNormal();

    getCursorPosition(&buttonPos[REF_TEMP_DOWN]);
    if (currentSelection == REF_TEMP_DOWN) setTextHighlight();
    printf(" [온도내리기] ");
    setTextNormal();

    if (refregiratorTemperature < 0)  printf("  냉장실 온도가 너무 낮습니다");
    if (refregiratorTemperature > 4) printf("  냉장실 온도가 너무 높습니다");

    printf("\n냉동실(%3d도)", freezerTemperature);

    getCursorPosition(&buttonPos[FRE_TEMP_UP]);
    if (currentSelection == FRE_TEMP_UP) setTextHighlight();
    printf(" [온도올리기] ");
    setTextNormal();

    getCursorPosition(&buttonPos[FRE_TEMP_DOWN]);
    if (currentSelection == FRE_TEMP_DOWN) setTextHighlight();
    printf(" [온도내리기] ");
    setTextNormal();

    if (refregiratorTemperature < -20) printf("  냉장실 온도가 너무 낮습니다");
    if (freezerTemperature > -10) printf("  냉동실 온도가 너무 높습니다");

    printReservedItem();

    getCursorPosition(&buttonPos[PREVIOUS]);
    if (currentSelection == PREVIOUS) setTextHighlight();
    if(buttonPos) printf("[이전화면으로]\n");
    setTextNormal();

    printf("\n위 아래 화살표 키로 커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n");
}

// 새로운 항목을 추가 해 줌 - 실제 기능은 addReservedItem() 함수가 시행
// data 무결성에 영향을 미치면 false를  return, 아니면 true를 return
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

    // 날짜를 console 에서 입력 받는다
    time_t inputDate = inputDateFromConsole("유통기한");
    if (inputDate == -1) {
        printf("\n 날자 입력에 실패 하였습니다, 아무 키나 누르시면 돌아갑니다");
        _getch();
        return true;
    }
    
    // 입력 받은 날자를 출력
    newItem.expire_date = inputDate;
    char* expDate = getFormatedStringByTime_t(&newItem.expire_date);
    printf("입력한 Expire date : %s\n", expDate);
    free(expDate);

    // 실제로 data를 reservedItem 에 저장 시도
    if (addReservedItem(&newItem) == false) { printf("Error!!! fail to add ReservedItem()"); exit(0); }

    printf("\n확인 하셨으면 y 키를 누르세요 >>");
    while (_getch() != 'y') {};

    return true;
}

// data 무결성에 영향을 미치면 false를  return, 아니면 true를 return
bool doDeleteItem() {
    clearConsole();
    printf("*****************************************\n");
    printf("삭제할 품목의 번호를 선택하고 엔터를 누르세요\n");
    printf("*****************************************\n");

    printReservedItem();

    int select;
    int countOfTry = 0;

    while (true) {
        if (countOfTry == 3) {
            printf("\n3번 이상 입력에 실패했습니다, 삭제 실패, 아무키나 누르시면 돌아 갑니다");
            return true;
        }

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

// data 무결성에 영향을 미치면 false를  return, 아니면 true를 return
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
        printf("%5d  %-20s  %3d개  %-30s  %-30s\n", i, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
        free(start_date);
        free(expire_date);
    }
    printf("------------------------------------------------------------------------------------------------\n");

    return true;
}

// time_t => YYYY-MM-DD 로 변환
// caller가 buff를 free 해 주어야 한다
// 실패하면 NULL을 return
char* getFormatedStringByTime_t(time_t* ttCurrent) {
    struct tm localTM;
    localtime_s(&localTM, ttCurrent);
    char* buff = (char*)malloc(sizeof(char)*256) ;

    if (buff == 0) {
        printf("Memory allocation error in getFormatedStringByTime_t,  아무키나 입력하세요\n");
        _getch();
        return NULL;
    }
    else {

        if (strftime(buff, sizeof(char)*256, "%Y년 %m월 %d일", &localTM) == 0) {
            printf("time_t를 문자열로 변경하는데 실패했습니다, 아무 키나 입력하세요 \n");
            _getch();
            free(buff);
            return NULL;
        }

        // 끝에 포함된 \n을 없앤다.
        size_t length = strlen(buff);
        if (length > 0 && buff[length - 1] == '\n') {
            buff[length - 1] = '\0'; 
        }

        return buff;
    }
}

// 실패하면 false 를 retuen
bool getCursorPosition(_CURSOR_POS* cursor_pos) {
    CONSOLE_SCREEN_BUFFER_INFO presentCur;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur) == 0) {
        printf("Error !!! GetConsoleScreenBufferInfo");
        return false;
    }
    else {
        cursor_pos->x = presentCur.dwCursorPosition.X;
        cursor_pos->y = presentCur.dwCursorPosition.Y;
        //printf("current cursor position  = %d, %d\n",cursor_pos->x, cursor_pos->y);
        return true;
    }
}


// 실패하면 false 를 retuen
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

// 실패하면 false 를 retuen
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

// 일단 처음에 5개 영역 확보
// 실패하면 false 를 retuen
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
// 실패하면 false 를 retuen
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
// data 무결성에 영향을 미치면 false를  return, 아니면 true를 return
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

// data 무결성에 영향을 미치면 false를  return, 아니면 true를 return
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

        printf("\n수정할 항목에 따라 단축기를 누르세요 : 품목이름-p,   갯수-c,  입고날자-s, 유통기한-e, 종료-q >> ");
        if (retry == 3) {
            printf("\nWarning!!! 3번 이상 잘 못 입력하셨습니다\n");
            _getch();
            return true;
        }


        selectOption = _getch();
        switch (selectOption) {
        // Enter key를 누르면 \n이 남으므로 buffer를 비움
        while (getchar() != '\n');
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
            reservedItem[select].count = newCount;
            retry = 0;
            break;
        case 's': 
            time_t newStartDate;
            newStartDate = inputDateFromConsole("새로운 입고 날자");
            if (newStartDate == -1) printf("\n날자 입력에 실패하셧어요, 다시 시도 하세요");
            else reservedItem[select].start_date = newStartDate;
            retry = 0;
            break;
        case 'e':
            time_t newExpireDate;
            newExpireDate = inputDateFromConsole("새로운 유통 기한");
            if (newExpireDate == -1) printf("\n날자 입력에 실패하셧어요, 다시 시도 하세요");
            else reservedItem[select].expire_date = newExpireDate;
            retry = 0;
            break;
        case 'q' :
            return true;
        default : 
            retry++;
            continue;
        }
    }

    return true;

}

// console로 부터 time_t구조를 입력 받아 검정 받은 후 return
// 실패하면 -1을 return
time_t inputDateFromConsole(const char* message ) {
    // 기본 tm 구조 선언
    struct tm inputTime = { 0 };
    time_t retTime_t;

    //  data  입력
    printf("\n%s을(를) 다음 형식과 같이 입력하세요 (YYYY-MM-DD) >> ", message);
    int retry = 0;
    while (true) {
        if (retry == 3) {
            printf("\n 3회 이상 입력에 실패 했습니다, 아무키나 누르시면 되돌아 갑니다");
            _getch();
            return -1;
        }
        else if (scanf_s("%d-%d-%d", &inputTime.tm_year, &inputTime.tm_mon, &inputTime.tm_mday) != 3) {
            printf("\n정확한 날짜 정보를 입력하세요 e.g 2023-11-14 Error %d회\n", retry);
            retry++;
            while (getchar() != '\n'); // input buffer를 비운다
            continue;
        }
        else {
            inputTime.tm_year -= 1900;
            inputTime.tm_mon--;
            retTime_t = mktime(&inputTime);
            if (retTime_t == -1) {
                printf("\n입력하신 날자가 유효한 날자가 아닙니다.  다시 입력하세요\n");
                retry++;
                continue;
            } else {
                return retTime_t;
            }
        }
    }
}

// 파일에 _ITEM* 저장
// 실패하면 message 보여주고 false를  return
bool doSaveToFile() {
    clearConsole();
    printf("---------------------------------------------\n");
    printf("파일로 저장");
    printf("---------------------------------------------\n");

    // binary 쓰기 모드
    FILE* filePointer;
    fopen_s(&filePointer, strFileName, "wb");

    if (filePointer == NULL) {
        printf("file open에 실패했습니다,  프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return false;
    }

    // fwrite ( data에 대한 pointer, size of item, count of item, file pointer )
    // countOfItem 을 쓴다 
    if (fwrite(&countOfItems, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return false;
    }

    // reservedItem 을 쓴다, 
    if (fwrite(reservedItem, sizeof(_ITEM), countOfItems, filePointer) != countOfItems) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return false;
    }

    fclose(filePointer);

    // 성공 메시지 표시
    printf("\n파일에서 데이터를 저장했습니다. 아무키나 누르세요 >>");
    _getch(); 

    return true;
}

// 파일에서 불러오기
// 쓸때 차지한  data를 잘 생각해서 복원
// 실패하면 message 보여주고 false를  return
bool doReadFromFile() {
    clearConsole();
    printf("---------------------------------------------\n");
    printf("파일에서 불러 오기");
    printf("---------------------------------------------\n");

    FILE* filePointer;

    if (fopen_s(&filePointer, strFileName, "rb") != 0) {
        printf("file 열기에 실패했습니다, 저장된 파일이 있는지 확인해 보세요, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return true;
    }

    // Read the count of items from the file
    if (fread(&countOfItems, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("file 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return false;
    }

    // Allocate memory for reservedItem based on the count read from the file
    free(reservedItem);
    reservedItem = (_ITEM*)malloc(sizeof(_ITEM) * countOfItems);

    // Read reservedItem from the file
    if (fread(reservedItem, sizeof(_ITEM), countOfItems, filePointer) != countOfItems) {
        fclose(filePointer);
        printf("file 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        free(reservedItem);
        return false;
    }

    fclose(filePointer);

    // 성공 메시지 표시
    printf("\n파일에서 데이터를 성공적으로 불러 왔습니다. 아무키나 누르세요 >>");
    _getch();

    return true;
}
