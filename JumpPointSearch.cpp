#include "JumpPointSearch.h"


DWORD g_num = 0;

CJumpPointSearch::CJumpPointSearch()
{
    m_startNode = new JPS_Node;
    m_endNode = new JPS_Node;
}

CJumpPointSearch::~CJumpPointSearch()
{
  
    //소멸자 만들어주기
}

void CJumpPointSearch::Initiate()
{
    //시작 노드 준비
    m_startNode->_startDistance = 0;
    m_startNode->_destDistance = CalcManhattanDist(m_startNode->_point, m_endPoint);
    m_startNode->_sumDistance = m_startNode->_destDistance;
    m_startNode->_directions = DIRECTION_ALL;
    m_startNode->_num = g_num++;

    //랜덤컬러 생성
    RandomColor();


}

void CJumpPointSearch::PrintNodeInTile()
{

    auto iter = m_closeList.begin();
    for (; iter != m_closeList.end(); ++iter)
    {
        auto node = *iter;
        m_tile[node.second->_point._y][node.second->_point._x] = NODE;
    }


    auto open_iter = m_openList.begin();
    for (; open_iter != m_openList.end(); ++open_iter)
    {
        auto node = *open_iter;
        m_tile[node.second->_point._y][node.second->_point._x] = NODE;
    }

    m_tile[m_startNode->_point._y][m_startNode->_point._x] = START;
    m_tile[m_endPoint._y][m_endPoint._x] = END;
}

bool CJumpPointSearch::Brasenham(Point srcPoint, Point dstPoint)
{
    Point tempPoint = srcPoint;
    int iNX, iNY, iDeltaX, iDeltaY, iDistance, iError = 0, iCount;
    WORD* wpDest;

    ////////////////////////////////////////////////////////////////////////////////
    // 시작점과, 끝점의 차를 구하여 직선의 X,Y 축의 변화량을 구한다.
    ////////////////////////////////////////////////////////////////////////////////
    iDeltaX = dstPoint._x - srcPoint._x;
    iDeltaY = dstPoint._y - srcPoint._y;

    ////////////////////////////////////////////////////////////////////////////////
    // 직선의 방향에 따라 선이 나아갈 좌표를 iNX, iNY 에 정한다.
    ////////////////////////////////////////////////////////////////////////////////
    iNX = (iDeltaX > 0) ? 1 : ((iDeltaX == 0) ? 0 : -1);
    iNY = (iDeltaY > 0) ? 1 : ((iDeltaY == 0) ? 0 : -1);

    ////////////////////////////////////////////////////////////////////////////////
    // 절대값을 구하여서..
    ////////////////////////////////////////////////////////////////////////////////
    iDeltaX = abs(iDeltaX);
    iDeltaY = abs(iDeltaY);

    ////////////////////////////////////////////////////////////////////////////////
    // X 축이 더 긴 경우와, Y축이 더 긴 경우 두가지 경우로 나뉜다.
    ////////////////////////////////////////////////////////////////////////////////
    if (iDeltaX > iDeltaY)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // X 축이 더 길으면, X 축을 기준으로...
        ////////////////////////////////////////////////////////////////////////////////
        iDistance = iDeltaX;

        for (iCount = iDistance; iCount; iCount--)
        {
            iError += iDeltaY;

            if (iError >= iDistance)
            {
                iError -= iDeltaX;
                tempPoint._y += iNY;
            }

            tempPoint._x += iNX;

            //여기에서 벽이 있는지 검사한다.
            if (m_tile[tempPoint._y][tempPoint._x] == OBSTACLE)
                return false;
            else if (m_tile[tempPoint._y][tempPoint._x] != NODE)
                m_tile[tempPoint._y][tempPoint._x] = REVISITED;


        }
    }
    else
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Y 축이 더 길으면, Y 축을 기준으로...
        ////////////////////////////////////////////////////////////////////////////////
        iDistance = iDeltaY;

        for (iCount = iDistance; iCount; iCount--)
        {
            iError += iDeltaX;


            if (iError >= iDistance)
            {
                iError -= iDeltaY;
                tempPoint._x += iNX;
            }

            tempPoint._y += iNY;

            //여기에서 벽이 있는지 검사한다.
            if (m_tile[tempPoint._y][tempPoint._x] == OBSTACLE)
                return false;
            else if(m_tile[tempPoint._y][tempPoint._x] != NODE)
                m_tile[tempPoint._y][tempPoint._x] = REVISITED;
        }

    }

    m_tile[m_startNode->_point._y][m_startNode->_point._x] = START;
    return true;

}

void CJumpPointSearch::ReFindPath()
{
    
    JPS_Node* parentNode = m_endNode->_pParent;
    JPS_Node* nowNode = m_endNode;
    JPS_Node* searchNode;
    JPS_Node* searchParentNode;
    while (parentNode != m_startNode && parentNode->_pParent != nullptr)
    {
        //거리 계산한다.
        int dist1 = CalcPitagorasDist(nowNode->_point, parentNode->_point) + CalcPitagorasDist(parentNode->_point, parentNode->_pParent->_point);
        int dist2 = CalcPitagorasDist(nowNode->_point, parentNode->_pParent->_point);

        if (dist2 < dist1)
        {
            //거리를 계산해서 조건에 맞으면 Brasenham 알고리즘으로 중간에 벽이 있는지 검사한다.
            if(Brasenham(nowNode->_point, parentNode->_pParent->_point))
                nowNode->_pParent = parentNode->_pParent;
        }

        //뒤에 노드들 가운데 F값이 더 작은 노드가 있다면 그친구를 부모로 만들어준다.
        searchNode = nowNode;
        searchParentNode = nowNode->_pParent;
        while (searchParentNode != m_startNode && searchParentNode->_pParent != nullptr)
        {
            if (searchParentNode->_sumDistance < searchNode->_pParent->_sumDistance)
            {
                if (Brasenham(searchNode->_point, searchParentNode->_point))
                    searchNode->_pParent = searchParentNode;
            }
            searchParentNode = searchParentNode->_pParent;
        }

        nowNode = nowNode->_pParent;        
        parentNode = parentNode->_pParent;

    }


}

//루프를 다 돌아도 못찾았을 시 예외처리 필요
void CJumpPointSearch::FindPath_Loop()
{
	JPS_Node* now_node = m_startNode;
    m_closeList[m_startNode->_num] = m_startNode;


    Point nextPoint;
	while (now_node != m_endNode)
	{

        if (now_node->_directions & DIRECTION_UU)
        {
            nextPoint = NewPoint_UU(now_node->_point);
            FindPath_Linear(now_node, nextPoint, UU);
        }

        if (now_node->_directions & DIRECTION_RU)
        {
            nextPoint = NewPoint_RU(now_node->_point);
            FindPath_Diagonal(now_node, nextPoint, RU);
        }

        if (now_node->_directions & DIRECTION_RR)
        {

            nextPoint = NewPoint_RR(now_node->_point);
            FindPath_Linear(now_node, nextPoint, RR);
        }
		
        if (now_node->_directions & DIRECTION_RD)
        {

            nextPoint = NewPoint_RD(now_node->_point);
            FindPath_Diagonal(now_node, nextPoint, RD);
        }
		
        if (now_node->_directions & DIRECTION_DD)
        {

            nextPoint = NewPoint_DD(now_node->_point);
            FindPath_Linear(now_node, nextPoint, DD);
        }
		
        if (now_node->_directions & DIRECTION_LD)
        {

            nextPoint = NewPoint_LD(now_node->_point);
            FindPath_Diagonal(now_node, nextPoint, LD);
        }

        if (now_node->_directions & DIRECTION_LL)
        {

            nextPoint = NewPoint_LL(now_node->_point);
            FindPath_Linear(now_node, nextPoint, LL);
        }

        if (now_node->_directions & DIRECTION_LU)
        {

            nextPoint = NewPoint_LU(now_node->_point);
            FindPath_Diagonal(now_node, nextPoint, LU);
        }


        //색 바꾸기
        RandomColor();

        //열린 목록이 비어있다면 종료
        if (m_openList.size() <= 0)
        {
            foundFlag = false;
            break;

        }
        //열린 목록에서 맨 앞 노드를 꺼내고 지워주기
        auto open_iter = m_openList.begin();
        auto pair = *open_iter;
        m_openList.erase(open_iter);
        now_node = pair.second;


        //닫힌 목록에 넣기
        m_closeList[now_node->_num] = now_node;

	}


    PrintNodeInTile();
    ReFindPath();


}

void CJumpPointSearch::FindPath_Linear(JPS_Node* startNode, Point nowPoint, BYTE direction)
{

    //다음 포인트
    Point next_point;

    //위쪽 벽
    Point wall_up;

    //아래쪽 벽
    Point wall_down;

    //위쪽 코너
    Point corner_up;

    //아래쪽 코너
    Point corner_down;

    //위쪽 코너 방향
    BYTE corner_up_dir;

    //아래쪽 코너 방향
    BYTE corner_down_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //총 방향 합산
    BYTE dir_sum = DIRECTION_NONE;


    switch (direction)
    {
    case UU:
        //위쪽 벽
        wall_up = NewPoint_RR(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_RU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_LL(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_LU(nowPoint);

        //다음 포인트
        next_point = NewPoint_UU(nowPoint);

        //코너 디렉션
        corner_up_dir = RU;
        corner_down_dir = LU;

        break;

    case RR:
        //위쪽 벽
        wall_up = NewPoint_UU(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_RU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_DD(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point = NewPoint_RR(nowPoint);


        //코너 디렉션
        corner_up_dir = RU;
        corner_down_dir = RD;

        break;
    case DD:
        //위쪽 벽
        wall_up = NewPoint_LL(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_LD(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_RR(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point = NewPoint_DD(nowPoint);


        //코너 디렉션
        corner_up_dir = LD;
        corner_down_dir = RD;

        break;
    case LL:
        //위쪽 벽
        wall_up = NewPoint_UU(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_LU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_DD(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_LD(nowPoint);

        //다음 포인트
        next_point = NewPoint_LL(nowPoint);


        //코너 디렉션
        corner_up_dir = LU;
        corner_down_dir = LD;

        break;
    default:
        CRASH();
        break;
    }

  


    //맵 최대 범위를 초과했을 때 중단
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return;

    //벽을 만나면 중단
    if (m_tile[yPos][xPos] == OBSTACLE)
        return;    

  
    //endpoint에 도달했을 경우
    //endNode를 설정하고
    //열린 목록에 endNode를 넣은 뒤 중단
    if (nowPoint == m_endPoint)
    {
        m_endNode->_point = m_endPoint;
        m_endNode->_pParent = startNode;
        m_endNode->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
        m_endNode->_destDistance = 0;
        m_endNode->_sumDistance = m_endNode->_startDistance;
        m_endNode->_num = g_num++;

        //열린 목록에 넣는다.
        m_openList.insert(make_pair(m_endNode->_sumDistance, m_endNode));

        return;
    }


    //코너 체크
    //1. 노드 기준 위아래가 막혀있는지 체크
    //2. 노드 대각선으로 막혀있지 않은지 체크
    if (m_tile[wall_up._y][wall_up._x] == OBSTACLE && m_tile[corner_up._y][corner_up._x] != OBSTACLE && CheckWalkable(corner_up))
        dir_sum += (DIRECTION_INIT << corner_up_dir);

    if (m_tile[wall_down._y][wall_down._x] == OBSTACLE && m_tile[corner_down._y][corner_down._x] != OBSTACLE && CheckWalkable(corner_down))
        dir_sum += (DIRECTION_INIT << corner_down_dir);


    if (dir_sum > DIRECTION_NONE)
    {
        if (m_tile[next_point._y][next_point._x] != OBSTACLE)
            dir_sum += (DIRECTION_INIT << direction);

        //오픈리스트에 있을 경우 g값만 변경해주기
        auto open_iter = m_openList.begin();
        for (; open_iter != m_openList.end(); ++open_iter)
        {
            auto node = *open_iter;
            if (node.second->_point == nowPoint)
            {
                float nowDist = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
                if (nowDist < node.second->_startDistance)
                {
                    node.second->_pParent = startNode;
                    node.second->_startDistance = nowDist;
                    node.second->_sumDistance = node.second->_startDistance + node.second->_destDistance;
                    node.second->_directions = dir_sum;
                }
                break;
            }
        }

        //클로즈리스트에 이미 있을 경우 중단
        auto close_iter = m_closeList.begin();
        for (; close_iter != m_closeList.end(); ++close_iter)
        {
            auto node = *close_iter;
            if (node.second = m_startNode)
                continue;

            if (node.second->_point == nowPoint)
            {
                float nowDist = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
                if (nowDist < node.second->_startDistance)
                {
                    node.second->_pParent = startNode;
                    node.second->_startDistance = nowDist;
                    node.second->_sumDistance = node.second->_startDistance + node.second->_destDistance;
                    node.second->_directions = dir_sum;
                }
                return;
            }
        }



        //새로운 노드를 생성
        JPS_Node* new_node = new JPS_Node;
        new_node->_pParent = startNode;
        new_node->_point = nowPoint;
        new_node->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, new_node->_point);
        new_node->_destDistance = CalcManhattanDist(new_node->_point, m_endPoint);
        new_node->_sumDistance = new_node->_startDistance + new_node->_destDistance;
        new_node->_directions = dir_sum;
        new_node->_num = g_num++;

        //생성한 노드를 열린 목록에 넣는다.
        m_openList.insert(make_pair(new_node->_sumDistance, new_node));


        
    }
    else 
    {
        //타일에 방문했다는 속성 체크를 해준다.
        if (m_tile[yPos][xPos] == EMPTY)
            m_tile[yPos][xPos] = VISITED;
        //타일색상을 저장한다.
        m_tileColor[yPos][xPos] = m_nowColor;
        //next point로 재귀한다.
        FindPath_Linear(startNode, next_point, direction);
    }
}

bool CJumpPointSearch::FindPath_Linear_SubSearch(Point nowPoint, BYTE direction)
{

    //다음 포인트
    Point next_point;

    //위쪽 벽
    Point wall_up;

    //아래쪽 벽
    Point wall_down;

    //위쪽 코너
    Point corner_up;

    //아래쪽 코너
    Point corner_down;

    //위쪽 코너 방향
    BYTE corner_up_dir;

    //아래쪽 코너 방향
    BYTE corner_down_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //총 방향 합산
    BYTE dir_sum = DIRECTION_NONE;


    switch (direction)
    {
    case UU:
        //위쪽 벽
        wall_up = NewPoint_RR(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_RU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_LL(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_LU(nowPoint);

        //다음 포인트
        next_point = NewPoint_UU(nowPoint);

        //코너 디렉션
        corner_up_dir = RU;
        corner_down_dir = LU;

        break;

    case RR:
        //위쪽 벽
        wall_up = NewPoint_UU(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_RU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_DD(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point = NewPoint_RR(nowPoint);


        //코너 디렉션
        corner_up_dir = RU;
        corner_down_dir = RD;

        break;
    case DD:
        //위쪽 벽
        wall_up = NewPoint_LL(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_LD(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_RR(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point = NewPoint_DD(nowPoint);


        //코너 디렉션
        corner_up_dir = LD;
        corner_down_dir = RD;

        break;
    case LL:
        //위쪽 벽
        wall_up = NewPoint_UU(nowPoint);

        //위쪽 코너
        corner_up = NewPoint_LU(nowPoint);

        //아래쪽 벽
        wall_down = NewPoint_DD(nowPoint);

        //아래쪽 코너
        corner_down = NewPoint_LD(nowPoint);

        //다음 포인트
        next_point = NewPoint_LL(nowPoint);


        //코너 디렉션
        corner_up_dir = LU;
        corner_down_dir = LD;

        break;
    default:
        CRASH();
        break;
    }

    //맵 최대 범위를 초과했을 때 중단
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return false;

    //벽을 만나면 중단
    if (m_tile[yPos][xPos] == OBSTACLE)
        return false;

    //endpoint에 도달했을 경우
    //이 탐색을 뿌린 위치에 노드 생성
    if (nowPoint == m_endPoint)
        return true;


    //코너 체크
    //1. 노드 기준 위아래가 막혀있는지 체크
    //2. 노드 대각선으로 막혀있지 않은지 체크
    //보조탐색이 맵 범위를 초과했을 때 중단
    if (m_tile[wall_up._y][wall_up._x] == OBSTACLE && m_tile[corner_up._y][corner_up._x] != OBSTACLE && CheckWalkable(corner_up))
        return true;

    if (m_tile[wall_down._y][wall_down._x] == OBSTACLE && m_tile[corner_down._y][corner_down._x] != OBSTACLE && CheckWalkable(corner_down))
        return true;


    //타일에 방문했다는 속성 체크를 해준다.
    m_tile[yPos][xPos] = VISITED;
    //타일색상을 저장한다.
    m_tileColor[yPos][xPos] = m_nowColor;
    //next point로 재귀한다.
    FindPath_Linear_SubSearch(next_point, direction);
    


}

void CJumpPointSearch::FindPath_Diagonal(JPS_Node* startNode, Point nowPoint, BYTE direction)
{

    //다음 포인트_3번
    Point next_point_3;
 
    //위쪽 벽
    Point wall_up_1;
    
    //아래쪽 벽
    Point wall_down_5;

    //위쪽 코너_1번
    Point corner_1;

    //아래쪽 코너_5번
    Point corner_5;

    //중간 공간_2번
    Point space_2;

    //중간 공간_4번
    Point space_4;

    //1번~5번 방향
    BYTE corner_1_dir;
    BYTE corner_2_dir;
    BYTE corner_3_dir_main;
    BYTE corner_4_dir;
    BYTE corner_5_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //총 방향 합산
    BYTE dir_sum = DIRECTION_NONE;

    switch (direction)
    {
    case RU:
        //위쪽 벽
        wall_up_1 = NewPoint_LL(nowPoint);

        //아래쪽 벽
        wall_down_5 = NewPoint_DD(nowPoint);

        //위쪽 코너
        corner_1 = NewPoint_LU(nowPoint);

        //아래쪽 코너
        corner_5 = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point_3 = NewPoint_RU(nowPoint);

        //빈공간 2번
        space_2 = NewPoint_UU(nowPoint);

        //빈공간 4번
        space_4 = NewPoint_RR(nowPoint);

        //코너 디렉션
        corner_1_dir = LU;
        corner_2_dir = UU;
        corner_3_dir_main = RU;
        corner_4_dir = RR;
        corner_5_dir = RD;


        break;

    case RD:
        //위쪽 벽
        wall_up_1 = NewPoint_UU(nowPoint);

        //아래쪽 벽
        wall_down_5 = NewPoint_LL(nowPoint);

        //위쪽 코너
        corner_1 = NewPoint_RU(nowPoint);

        //아래쪽 코너
        corner_5 = NewPoint_LD(nowPoint);

        //다음 포인트
        next_point_3 = NewPoint_RD(nowPoint);

        //빈공간 2번
        space_2 = NewPoint_RR(nowPoint);

        //빈공간 4번
        space_4 = NewPoint_DD(nowPoint);

        //코너 디렉션
        corner_1_dir = RU;
        corner_2_dir = RR;
        corner_3_dir_main = RD;
        corner_4_dir = DD;
        corner_5_dir = LD;
        break;

    case LD:

        //위쪽 벽
        wall_up_1 = NewPoint_UU(nowPoint);

        //아래쪽 벽
        wall_down_5 = NewPoint_RR(nowPoint);

        //위쪽 코너
        corner_1 = NewPoint_LU(nowPoint);

        //아래쪽 코너
        corner_5 = NewPoint_RD(nowPoint);

        //다음 포인트
        next_point_3 = NewPoint_LD(nowPoint);

        //빈공간 2번
        space_2 = NewPoint_LL(nowPoint);

        //빈공간 4번
        space_4 = NewPoint_DD(nowPoint);
        //코너 디렉션
        corner_1_dir = LU;
        corner_2_dir = LL;
        corner_3_dir_main = LD;
        corner_4_dir = DD;
        corner_5_dir = RD;
        break;

    case LU:

        //위쪽 벽
        wall_up_1 = NewPoint_RR(nowPoint);

        //아래쪽 벽
        wall_down_5 = NewPoint_DD(nowPoint);

        //위쪽 코너
        corner_1 = NewPoint_RU(nowPoint);

        //아래쪽 코너
        corner_5 = NewPoint_LD(nowPoint);

        //다음 포인트
        next_point_3 = NewPoint_LU(nowPoint);

        //빈공간 2번
        space_2 = NewPoint_UU(nowPoint);

        //빈공간 4번
        space_4 = NewPoint_LL(nowPoint);

        //코너 디렉션
        corner_1_dir = RU;
        corner_2_dir = UU;
        corner_3_dir_main = LU;
        corner_4_dir = LL;
        corner_5_dir = LD;
        break;

    default:
        CRASH();
        break;
    }

    //맵 최대 범위를 초과했을 때 중단
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return;

    //벽을 만나면 중단
    if (m_tile[yPos][xPos] == OBSTACLE)
        return;

 
    //endpoint에 도달했을 경우
    //endNode를 설정하고
    //열린 목록에 endNode를 넣은 뒤 중단
    if (nowPoint == m_endPoint)
    {
        m_endNode->_point = m_endPoint;
        m_endNode->_pParent = startNode;
        m_endNode->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
        m_endNode->_destDistance = 0;
        m_endNode->_sumDistance = m_endNode->_startDistance;
        m_endNode->_num = g_num++;

        //열린 목록에 넣는다.
        m_openList.insert(make_pair(m_endNode->_sumDistance, m_endNode));
        return;
    }

    //코너 체크
    //1. 진행방향 뒤쪽 벽 2개가 막혀있는지 체크
    //2. 노드 옆으로 막혀있지 않은지 체크
    //3. 코너와 원래 진행방향 사이가 막혀있지 않은지 체크(보조탐색)

    if (m_tile[wall_up_1._y][wall_up_1._x] == OBSTACLE && m_tile[corner_1._y][corner_1._x] != OBSTACLE && CheckWalkable(corner_1))
        dir_sum += (DIRECTION_INIT << corner_1_dir);

    if (m_tile[wall_down_5._y][wall_down_5._x] == OBSTACLE && m_tile[corner_5._y][corner_5._x] != OBSTACLE && CheckWalkable(corner_5))
        dir_sum += (DIRECTION_INIT << corner_5_dir);

    if (FindPath_Linear_SubSearch(space_2, corner_2_dir))
        dir_sum += (DIRECTION_INIT << corner_2_dir);

    if (FindPath_Linear_SubSearch(space_4, corner_4_dir))
        dir_sum += (DIRECTION_INIT << corner_4_dir);


    if (dir_sum > DIRECTION_NONE)
    {
        if (m_tile[next_point_3._y][next_point_3._x] != OBSTACLE)
            dir_sum += (DIRECTION_INIT << direction);

        //오픈리스트에 있을 경우 g값만 변경해주기
        auto open_iter = m_openList.begin();
        for (; open_iter != m_openList.end(); ++open_iter)
        {
            auto node = *open_iter;
            if (node.second->_point == nowPoint)
            {
                float nowDist = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
                if (nowDist < node.second->_startDistance)
                {
                    node.second->_pParent = startNode;
                    node.second->_startDistance = nowDist;
                    node.second->_sumDistance = node.second->_startDistance + node.second->_destDistance;
                    node.second->_directions = dir_sum;
                }
                break;
            }
        }

        //클로즈리스트에 이미 있을 경우 중단
        auto close_iter = m_closeList.begin();
        for (; close_iter != m_closeList.end(); ++close_iter)
        {
            auto node = *close_iter;
            if (node.second = m_startNode)
                continue;

            if (node.second->_point == nowPoint)
            {
                float nowDist = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
                if (nowDist < node.second->_startDistance)
                {
                    node.second->_pParent = startNode;
                    node.second->_startDistance = nowDist;
                    node.second->_sumDistance = node.second->_startDistance + node.second->_destDistance;
                    node.second->_directions = dir_sum;
                }
                return;
            }
        }


        
        //새로운 노드를 생성
        JPS_Node* new_node = new JPS_Node;
        new_node->_pParent = startNode;
        new_node->_point = nowPoint;
        new_node->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, new_node->_point);
        new_node->_destDistance = CalcManhattanDist(new_node->_point, m_endPoint);
        new_node->_sumDistance = new_node->_startDistance + new_node->_destDistance;
        new_node->_directions = dir_sum;
        new_node->_num = g_num++;

        //생성한 노드를 열린 목록에 넣는다.
        m_openList.insert(make_pair(new_node->_sumDistance, new_node));

  
    }
    else
    {
        //타일에 방문했다는 속성 체크를 해준다.
        if(m_tile[yPos][xPos] == EMPTY)
            m_tile[yPos][xPos] = VISITED;
        //타일색상을 저장한다.
        m_tileColor[yPos][xPos] = m_nowColor;
        //next point로 재귀한다.
        FindPath_Diagonal(startNode, next_point_3, direction);
    }

}

void CJumpPointSearch::Clear()
{
   

    //리스트 초기화
    auto openIter = m_openList.begin();
    for (; openIter != m_openList.end();)
    {
        auto pair = *openIter;
        delete pair.second;
        openIter = m_openList.erase(openIter);
    }

    auto closeIter = m_closeList.begin();
    for (; closeIter != m_closeList.end();)
    {
        auto pair = *closeIter;
        delete pair.second;
        closeIter = m_closeList.erase(closeIter);
    }

    m_openList.clear();
    m_closeList.clear();

    //시작, 종단 노드 재생성
    m_startNode = new JPS_Node;
    m_endNode = new JPS_Node;

    //시작 노드 초기화
    m_startNode->_destDistance = 0;
    m_startNode->_point._x = 0;
    m_startNode->_point._y = 0;
    m_startNode->_sumDistance = 0;

    //종단 노드 초기화
    m_startNode->_destDistance = 0;
    m_startNode->_point._x = 0;
    m_startNode->_point._y = 0;
    m_startNode->_sumDistance = 0;

    //종단 포인트 초기화
    m_endPoint._x = 0;
    m_endPoint._y = 0;

    //2차원 배열 초기화
    ZeroMemory(m_tile, GRID_HEIGHT * GRID_WIDTH);
    ZeroMemory(m_tileColor, GRID_HEIGHT * GRID_WIDTH * sizeof(Color));

    //번호 초기화
    g_num = 0;
}

void CJumpPointSearch::RandomColor()
{
    //색 재설정
    m_nowColor._red = rand() % 255;
    m_nowColor._green = rand() % 255;
    m_nowColor._blue = rand() % 255;
}

void CJumpPointSearch::RandomTile()
{

    for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
    {
        for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
        {
            bool random = rand() % 2;
            m_tile[iCntH][iCntW] = random;
        }
    }
}

bool CJumpPointSearch::CheckWalkable(Point point)
{
    if (point._x < 0 || point._y < 0 || point._x >= GRID_WIDTH || point._y >= GRID_HEIGHT || m_tile[point._y][point._x] == OBSTACLE)
        return false;
    else
        return true;
}
