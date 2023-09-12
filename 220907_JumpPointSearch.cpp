// 220902_A_star_algorithm.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "JumpPointSearch.h"
#include "220907_JumpPointSearch.h"

#define MAX_LOADSTRING 100

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif


HWND hWnd;


constexpr int DEFAULT_GRID_SIZE = 20;
constexpr int DIRECTION_SIZE = 8;
int GRID_SIZE = DEFAULT_GRID_SIZE;


//브러쉬
HBRUSH g_hTileBrush;
HBRUSH g_hStartBrush;
HBRUSH g_hEndBrush;
HBRUSH g_hNodeBrush;
HBRUSH g_hRandomBrush;
HBRUSH g_hRevisitedBrush;

//그리드, 시작, 끝, 오픈리스트, 클로즈리스트 펜
HPEN g_hGridPen;
HPEN g_hLinePen;
HPEN g_hEndPen;
//
////타일을 표현하는 2차원 배열
//char g_Tile[GRID_HEIGHT][GRID_WIDTH];
////0 장애물 없음/ 1 장애물 있음
//
////타일 색깔을 표현하는 2차원 배열
//Color g_TileColor[GRID_HEIGHT][GRID_WIDTH];

//char* g_oldStartTile;
//
//JPS_Node* g_startNode;
//JPS_Node* g_endNode;
//Point g_endPoint;

//시작, 끝 타일
char* g_startTile;
char* g_endTile;

//더블버퍼링용 전역 변수들
HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;
HDC g_hMemDC;
RECT g_MemDCRect;

//타일의 속성 입력/제거 모드 플래그
//클릭클릭시 해당 타일의 속성을 단순 반전만 시켜준다면 
//드래그를 통해서 장애물 입력을 수월하게 하기 어렵다.
//첫 클릭 시 해당 타일의 속성에 따라서 장애물 입력 모드, 제거 모드를 본 플래그로 지정한다.
//마우스 이동 시 지정된 방법으로 장애물을 연속적으로 세팅한다.

bool g_bErase = false;
bool g_bDrag = false;

//선 그리기 플래그
bool g_bLineFlag = false;

//시작 타일 플래그
bool g_startFlag = false;

//종단 타일 플래그
bool g_endFlag = false;

//중복 길찾기 방지 플래그
bool g_pathFindDone = false;

CJumpPointSearch g_JPS;


void RenderGrid(HDC hdc)
{
    int iX = 0;
    int iY = 0;
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

    //그리드의 마지막 선을 추가로 그리기 위해 <= 의 반복 조건
    for (int iCntW = 0; iCntW <= GRID_WIDTH; iCntW++)
    {
        MoveToEx(hdc, iX, 0, NULL);
        LineTo(hdc, iX, GRID_HEIGHT * GRID_SIZE);
        iX += GRID_SIZE;
    }
    for (int iCntH = 0; iCntH <= GRID_HEIGHT; iCntH++)
    {
        MoveToEx(hdc, 0, iY, NULL);
        LineTo(hdc, GRID_WIDTH * GRID_SIZE, iY);
        iY += GRID_SIZE;
    }

    SelectObject(hdc, hOldPen);
}

void RenderObstacle(HDC hdc)
{
    int iX = 0;
    int iY = 0;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));

    //사각형의 테두리를 보이지 않도록 하기 위해 NULL_PEN을 지정한다.
    //CreatePen으로 NULL_PEN을 생성해도 되지만
    //GetStockObject를 사용하여 이미 시스템에 만들어져있는 고정 GDI Object를 사용해본다.
    //GetStockObejct는 시스템의 고정적인 범용 GDI Object로서 삭제가 필요하지 않다.
    //시스템 전역적인 GDI Object를 얻어서 사용한다는 개념이다.

    for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
    {
        for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
        {

            switch (g_JPS.m_tile[iCntH][iCntW])
            {
            case EMPTY:
                continue;
            case OBSTACLE:
                break;
            case START:
                SelectObject(hdc, g_hStartBrush);
                break;
            case END:
                SelectObject(hdc, g_hEndBrush);
                break;
            case VISITED:
            {
                Color color = g_JPS.m_tileColor[iCntH][iCntW];
                g_hRandomBrush = CreateSolidBrush(RGB(color._red, color._green, color._red));
                SelectObject(hdc, g_hRandomBrush);
            }
                break;
            case NODE:
                SelectObject(hdc, g_hNodeBrush);
                break;
            case REVISITED:
                SelectObject(hdc, g_hRevisitedBrush);
                break;
            }


            iX = iCntW * GRID_SIZE;
            iY = iCntH * GRID_SIZE;
            //테두리 크기가 있으므로 +2한다.
            Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);

            if (g_JPS.m_tile[iCntH][iCntW] == NODE || g_JPS.m_tile[iCntH][iCntW] == START || g_JPS.m_tile[iCntH][iCntW] == END)
            {
                WCHAR BUF[16];
                int cipher = 0;
                _itow_s(g_JPS.m_tileNumber[iCntH][iCntW], BUF, 10);      
                if (g_JPS.m_tileNumber[iCntH][iCntW] >= 10)
                    cipher = 2;
                else
                    cipher = 1;
                TextOutW(hdc, iX + 6, iY + 5, BUF, cipher);
            }
            SelectObject(hdc, g_hTileBrush);
        }
    }
    SelectObject(hdc, hOldBrush);
}

void RenderLine(HDC hdc)
{
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hLinePen);

    auto now_tile = g_JPS.m_endNode;
    auto next_tile = g_JPS.m_endNode->_pParent;

    wprintf(L"------------------------- \n");
    wprintf(L"최종 경로 \n");
    wprintf(L"------------------------- \n");

    DWORD count = 1;

    while (now_tile != g_JPS.m_startNode)
    {
        wprintf(L"------------------------- \n");
        wprintf(L"No. %d - X : %d, Y : %d \n", g_JPS.m_tileNumber[now_tile->_point._y][now_tile->_point._x], now_tile->_point._x, now_tile->_point._y);
        wprintf(L"No. %d - G : %f, H : %f, F : %lf \n", g_JPS.m_tileNumber[now_tile->_point._y][now_tile->_point._x], now_tile->_startDistance, now_tile->_destDistance, now_tile->_sumDistance);

        if (now_tile->_directions & DIRECTION_UU)
            wprintf(L"UU \n");

        if (now_tile->_directions & DIRECTION_RU)
            wprintf(L"RU \n");

        if (now_tile->_directions & DIRECTION_RR)
            wprintf(L"RR \n");

        if (now_tile->_directions & DIRECTION_RD)
            wprintf(L"RD \n");

        if (now_tile->_directions & DIRECTION_DD)
            wprintf(L"DD \n");

        if (now_tile->_directions & DIRECTION_LD)
            wprintf(L"LD \n");

        if (now_tile->_directions & DIRECTION_LL)
            wprintf(L"LL \n");

        if (now_tile->_directions & DIRECTION_LU)
            wprintf(L"LU \n");

        next_tile = now_tile->_pParent;

        int now_iX = now_tile->_point._x * GRID_SIZE + (GRID_SIZE / 2);
        int now_iY = now_tile->_point._y * GRID_SIZE + (GRID_SIZE / 2);
        int next_iX = next_tile->_point._x * GRID_SIZE + (GRID_SIZE / 2);
        int next_iY = next_tile->_point._y * GRID_SIZE + (GRID_SIZE / 2);

        MoveToEx(hdc, now_iX, now_iY, NULL);
        LineTo(hdc, next_iX, next_iY);
        now_tile = next_tile;

        count++;
    }


    SelectObject(hdc, hOldPen);

}

void PrintNode()
{

    wprintf(L"------------------------- \n");
    wprintf(L"Open List \n");
    wprintf(L"------------------------- \n");
    auto open_iter = g_JPS.m_openList.begin();


    for (; open_iter != g_JPS.m_openList.end(); ++open_iter)
    {
        auto pair = *open_iter;

        wprintf(L"------------------------- \n");
        wprintf(L"No. %d - X : %d, Y : %d \n", pair.second->_num, pair.second->_point._x, pair.second->_point._y);
        wprintf(L"No. %d - G : %f, H : %f, F : %lf \n", pair.second->_num, pair.second->_startDistance, pair.second->_destDistance, pair.second->_sumDistance);
        wprintf(L"DIRECTIONS: \n");

        if (pair.second->_directions & DIRECTION_UU)
            wprintf(L"UU \n");

        if (pair.second->_directions & DIRECTION_RU)
            wprintf(L"RU \n");

        if (pair.second->_directions & DIRECTION_RR)
            wprintf(L"RR \n");

        if (pair.second->_directions & DIRECTION_RD)
            wprintf(L"RD \n");

        if (pair.second->_directions & DIRECTION_DD)
            wprintf(L"DD \n");

        if (pair.second->_directions & DIRECTION_LD)
            wprintf(L"LD \n");

        if (pair.second->_directions & DIRECTION_LL)
            wprintf(L"LL \n");

        if (pair.second->_directions & DIRECTION_LU)
            wprintf(L"LU \n");
        
        g_JPS.m_tileNumber[pair.second->_point._y][pair.second->_point._x] = pair.second->_num;
    }

    wprintf(L"------------------------- \n");
    wprintf(L"Close List \n");
    wprintf(L"------------------------- \n");

    auto close_iter = g_JPS.m_closeList.begin();
    for (; close_iter != g_JPS.m_closeList.end(); ++close_iter)
    {
        auto node = *close_iter;

        wprintf(L"------------------------- \n");
        wprintf(L"No. %d - X : %d, Y : %d \n", node.second->_num, node.second->_point._x, node.second->_point._y);
        wprintf(L"No. %d - G : %f, H : %f, F : %lf \n", node.second->_num, node.second->_startDistance, node.second->_destDistance, node.second->_sumDistance);
        wprintf(L"DIRECTIONS: \n");

        if (node.second->_directions & DIRECTION_UU)
            wprintf(L"UU \n");

        if (node.second->_directions & DIRECTION_RU)
            wprintf(L"RU \n");

        if (node.second->_directions & DIRECTION_RR)
            wprintf(L"RR \n");

        if (node.second->_directions & DIRECTION_RD)
            wprintf(L"RD \n");

        if (node.second->_directions & DIRECTION_DD)
            wprintf(L"DD \n");

        if (node.second->_directions & DIRECTION_LD)
            wprintf(L"LD \n");

        if (node.second->_directions & DIRECTION_LL)
            wprintf(L"LL \n");

        if (node.second->_directions & DIRECTION_LU)
            wprintf(L"LU \n");


        g_JPS.m_tileNumber[node.second->_point._y][node.second->_point._x] = node.second->_num;
    }



}

//=================================================================================

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY220907JUMPPOINTSEARCH, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY220907JUMPPOINTSEARCH));

    MSG msg;



    srand(22222);
    
    

    // 기본 메시지 루프입니다:
     while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY220907JUMPPOINTSEARCH));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY220907JUMPPOINTSEARCH);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    setlocale(LC_ALL, "");

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    //wParam - 마우스 이동 시 눌려진 키, 버튼, 버튼 눌림 여부를 bit단위로 넣어준다.
    //lParam - 마우스 좌표, low-order word the x-coordinate / high-order word the y-coordinate
    //lParam에는 X, Y좌표가 들어온다. Low Word/High Word 2바이트씩 나누어서 들어옴

    PAINTSTRUCT ps;  
    switch (message)
    {
    case WM_LBUTTONDOWN:
        g_bLineFlag = false;
        g_bDrag = true;
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = xPos / GRID_SIZE;
            int iTileY = yPos / GRID_SIZE;

            //첫 선택 타일이 장애물이면 지우기모드 
            //장애물 넣기 모드

            if (g_JPS.m_tile[iTileY][iTileX] == OBSTACLE)
                g_bErase = true;
            else
                g_bErase = false;

            InvalidateRect(hWnd, NULL, false);
            UpdateWindow(hWnd);
        }
        break;

    case WM_LBUTTONUP:
    {
        g_bDrag = false;
        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);

    }
        break;

    case WM_RBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int iTileX = xPos / GRID_SIZE;
        int iTileY = yPos / GRID_SIZE;

        if (g_JPS.m_tile[iTileY][iTileX] == OBSTACLE)
            break;

        if (!g_endFlag)
        {
            g_endFlag = true;
            g_endTile = &(g_JPS.m_tile[iTileY][iTileX]);
            g_JPS.m_endPoint._x = iTileX;
            g_JPS.m_endPoint._y = iTileY;
            *g_endTile = END;
        }
        else
        {
            g_endFlag = false;
            g_JPS.m_endPoint._x = 0;
            g_JPS.m_endPoint._y = 0;
            *g_endTile = EMPTY;
            g_endTile = nullptr;
        }

        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);

    }
    break;

    case WM_MOUSEMOVE:
    {
        if (g_bDrag)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = xPos / GRID_SIZE;
            int iTileY = yPos / GRID_SIZE;

            if (g_JPS.m_tile[iTileY][iTileX] != START)
                g_JPS.m_tile[iTileY][iTileX] = !g_bErase;


            InvalidateRect(hWnd, NULL, false);
            UpdateWindow(hWnd);
        }
    }
    break;

    case WM_KEYDOWN:
    {
        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);
        HDC hdcC = GetDC(hWnd);
        if (wParam == VK_UP && g_endTile != nullptr && g_startTile != nullptr)
        {
            if (!g_pathFindDone)
            {

                g_JPS.Initiate();
                g_JPS.FindPath_Loop();
                g_pathFindDone = true;
            }
            g_bLineFlag = true;
        }

        if (wParam == VK_DOWN)
        {
            g_JPS.RandomTile();
        }

        if (wParam == VK_SPACE)
        {
            g_JPS.Clear();
            g_pathFindDone = false;
        }

        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);
        ReleaseDC(hWnd, hdcC);
    }
    break;

    case WM_MBUTTONDOWN:
    {

        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int iTileX = xPos / GRID_SIZE;
        int iTileY = yPos / GRID_SIZE;

        if (g_JPS.m_tile[iTileY][iTileX] == OBSTACLE)
            break;

        if (!g_startFlag)
        {
            g_startFlag = true;
            g_startTile = &(g_JPS.m_tile[iTileY][iTileX]);
            g_JPS.m_startNode->_point._x = iTileX;
            g_JPS.m_startNode->_point._y = iTileY;
            *g_startTile = START;
        }
        else
        {
            g_startFlag = false;
            g_JPS.m_startNode->_point._x = 0;
            g_JPS.m_startNode->_point._y = 0;
            *g_startTile = EMPTY;
            g_startTile = nullptr;
        }

        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);
    }
    break;

    case WM_MBUTTONUP:
    {

        if (g_startTile)
        {

            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = xPos / GRID_SIZE;
            int iTileY = yPos / GRID_SIZE;

            if (&(g_JPS.m_tile[iTileY][iTileX]) == g_startTile)
            {
                *g_startTile = START;

                g_JPS.m_startNode->_point._x = iTileX;
                g_JPS.m_startNode->_point._y = iTileY;
            }

            InvalidateRect(hWnd, NULL, false);
            UpdateWindow(hWnd);
        }
    }
    break;

    case WM_MOUSEWHEEL:
    {
        if ((SHORT)HIWORD(wParam) > 0)
        {
            GRID_SIZE = 48;
        }
        else
        {
            GRID_SIZE = DEFAULT_GRID_SIZE;
        }


        InvalidateRect(hWnd, NULL, false);
        UpdateWindow(hWnd);
    }
    break;

    case WM_CREATE:
    {

        //윈도우 생성 시 현 윈도우 크기와 동일한 메모리DC 생성
        //스크롤 기능도 넣어보자..
        HDC hdcC = GetDC(hWnd);
        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdcC, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdcC);
        ReleaseDC(hWnd, hdcC);
        g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);

        g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        g_hLinePen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        g_hEndPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));



        g_hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
        g_hStartBrush = CreateSolidBrush(RGB(255, 0, 0));
        g_hEndBrush = CreateSolidBrush(RGB(0, 255, 0));
        g_hNodeBrush = CreateSolidBrush(RGB(255, 255, 0));
        g_hRevisitedBrush = CreateSolidBrush(RGB(0, 0, 0));

    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    //기존에는 윈도우DC, hdc를 대상으로 출력하였으나
    //이제는 메모리DC를 대상으로 출력한다.
    case WM_PAINT:
    {
        //메모리 DC를 클리어하고
        PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);


        if (g_bLineFlag)
        {
            //콘솔에 노드 정보를 출력
            PrintNode();
        }

        //RenderObstacle, RenderGrid를 메모리 DC에 출력
        RenderObstacle(g_hMemDC);
        RenderGrid(g_hMemDC);

        if (g_bLineFlag && g_JPS.foundFlag == true)
        {
            
            RenderLine(g_hMemDC);
            g_bLineFlag = false;
        }







        //메모리DC에 렌더링이 끝나면 메모리 DC -> 윈도우 DC로의 출력
        //DC에서 DC로의 출력 함수(대상DC, 출력좌표 x y, 출력크기 cx, cy, 소스 DC, 소스좌표 x y, 출력방법
        //를 인자로 넣어 출력가능하다.
        HDC hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        ReleaseDC(hWnd, hdc);
    }
    break;
    case WM_SIZE:
    {
        SelectObject(g_hMemDC, g_hMemDCBitmap_old);
        DeleteObject(g_hMemDC);
        DeleteObject(g_hMemDCBitmap);

        HDC hdcS = GetDC(hWnd);
        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdcS, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdcS);
        ReleaseDC(hWnd, hdcS);

        g_hMemDCBitmap = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
        PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);
    }
    break;
    case WM_DESTROY:
        SelectObject(g_hMemDC, g_hMemDCBitmap_old);
        DeleteObject(g_hMemDCBitmap);
        DeleteObject(g_hMemDCBitmap_old);
        DeleteObject(g_hMemDC);
        DeleteObject(g_hTileBrush);
        DeleteObject(g_hGridPen);

        //WM_QUIT메시지를 발생시키는 함수
        //메시지 루프의 종료를 유도한다.
        //#PostMessage() 함수도 존재한다. 임의의 메시지가 발생된 것처럼 임의로 메시지를 생성시킨다.
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
