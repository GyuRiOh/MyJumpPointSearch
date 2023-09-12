#include "JumpPointSearch.h"


DWORD g_num = 0;

CJumpPointSearch::CJumpPointSearch()
{
    m_startNode = new JPS_Node;
    m_endNode = new JPS_Node;
}

CJumpPointSearch::~CJumpPointSearch()
{
  
    //�Ҹ��� ������ֱ�
}

void CJumpPointSearch::Initiate()
{
    //���� ��� �غ�
    m_startNode->_startDistance = 0;
    m_startNode->_destDistance = CalcManhattanDist(m_startNode->_point, m_endPoint);
    m_startNode->_sumDistance = m_startNode->_destDistance;
    m_startNode->_directions = DIRECTION_ALL;
    m_startNode->_num = g_num++;

    //�����÷� ����
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
    // ��������, ������ ���� ���Ͽ� ������ X,Y ���� ��ȭ���� ���Ѵ�.
    ////////////////////////////////////////////////////////////////////////////////
    iDeltaX = dstPoint._x - srcPoint._x;
    iDeltaY = dstPoint._y - srcPoint._y;

    ////////////////////////////////////////////////////////////////////////////////
    // ������ ���⿡ ���� ���� ���ư� ��ǥ�� iNX, iNY �� ���Ѵ�.
    ////////////////////////////////////////////////////////////////////////////////
    iNX = (iDeltaX > 0) ? 1 : ((iDeltaX == 0) ? 0 : -1);
    iNY = (iDeltaY > 0) ? 1 : ((iDeltaY == 0) ? 0 : -1);

    ////////////////////////////////////////////////////////////////////////////////
    // ���밪�� ���Ͽ���..
    ////////////////////////////////////////////////////////////////////////////////
    iDeltaX = abs(iDeltaX);
    iDeltaY = abs(iDeltaY);

    ////////////////////////////////////////////////////////////////////////////////
    // X ���� �� �� ����, Y���� �� �� ��� �ΰ��� ���� ������.
    ////////////////////////////////////////////////////////////////////////////////
    if (iDeltaX > iDeltaY)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // X ���� �� ������, X ���� ��������...
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

            //���⿡�� ���� �ִ��� �˻��Ѵ�.
            if (m_tile[tempPoint._y][tempPoint._x] == OBSTACLE)
                return false;
            else if (m_tile[tempPoint._y][tempPoint._x] != NODE)
                m_tile[tempPoint._y][tempPoint._x] = REVISITED;


        }
    }
    else
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Y ���� �� ������, Y ���� ��������...
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

            //���⿡�� ���� �ִ��� �˻��Ѵ�.
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
        //�Ÿ� ����Ѵ�.
        int dist1 = CalcPitagorasDist(nowNode->_point, parentNode->_point) + CalcPitagorasDist(parentNode->_point, parentNode->_pParent->_point);
        int dist2 = CalcPitagorasDist(nowNode->_point, parentNode->_pParent->_point);

        if (dist2 < dist1)
        {
            //�Ÿ��� ����ؼ� ���ǿ� ������ Brasenham �˰������� �߰��� ���� �ִ��� �˻��Ѵ�.
            if(Brasenham(nowNode->_point, parentNode->_pParent->_point))
                nowNode->_pParent = parentNode->_pParent;
        }

        //�ڿ� ���� ��� F���� �� ���� ��尡 �ִٸ� ��ģ���� �θ�� ������ش�.
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

//������ �� ���Ƶ� ��ã���� �� ����ó�� �ʿ�
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


        //�� �ٲٱ�
        RandomColor();

        //���� ����� ����ִٸ� ����
        if (m_openList.size() <= 0)
        {
            foundFlag = false;
            break;

        }
        //���� ��Ͽ��� �� �� ��带 ������ �����ֱ�
        auto open_iter = m_openList.begin();
        auto pair = *open_iter;
        m_openList.erase(open_iter);
        now_node = pair.second;


        //���� ��Ͽ� �ֱ�
        m_closeList[now_node->_num] = now_node;

	}


    PrintNodeInTile();
    ReFindPath();


}

void CJumpPointSearch::FindPath_Linear(JPS_Node* startNode, Point nowPoint, BYTE direction)
{

    //���� ����Ʈ
    Point next_point;

    //���� ��
    Point wall_up;

    //�Ʒ��� ��
    Point wall_down;

    //���� �ڳ�
    Point corner_up;

    //�Ʒ��� �ڳ�
    Point corner_down;

    //���� �ڳ� ����
    BYTE corner_up_dir;

    //�Ʒ��� �ڳ� ����
    BYTE corner_down_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //�� ���� �ջ�
    BYTE dir_sum = DIRECTION_NONE;


    switch (direction)
    {
    case UU:
        //���� ��
        wall_up = NewPoint_RR(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_RU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_LL(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_LU(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_UU(nowPoint);

        //�ڳ� �𷺼�
        corner_up_dir = RU;
        corner_down_dir = LU;

        break;

    case RR:
        //���� ��
        wall_up = NewPoint_UU(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_RU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_DD(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_RR(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = RU;
        corner_down_dir = RD;

        break;
    case DD:
        //���� ��
        wall_up = NewPoint_LL(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_LD(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_RR(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_DD(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = LD;
        corner_down_dir = RD;

        break;
    case LL:
        //���� ��
        wall_up = NewPoint_UU(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_LU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_DD(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_LD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_LL(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = LU;
        corner_down_dir = LD;

        break;
    default:
        CRASH();
        break;
    }

  


    //�� �ִ� ������ �ʰ����� �� �ߴ�
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return;

    //���� ������ �ߴ�
    if (m_tile[yPos][xPos] == OBSTACLE)
        return;    

  
    //endpoint�� �������� ���
    //endNode�� �����ϰ�
    //���� ��Ͽ� endNode�� ���� �� �ߴ�
    if (nowPoint == m_endPoint)
    {
        m_endNode->_point = m_endPoint;
        m_endNode->_pParent = startNode;
        m_endNode->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
        m_endNode->_destDistance = 0;
        m_endNode->_sumDistance = m_endNode->_startDistance;
        m_endNode->_num = g_num++;

        //���� ��Ͽ� �ִ´�.
        m_openList.insert(make_pair(m_endNode->_sumDistance, m_endNode));

        return;
    }


    //�ڳ� üũ
    //1. ��� ���� ���Ʒ��� �����ִ��� üũ
    //2. ��� �밢������ �������� ������ üũ
    if (m_tile[wall_up._y][wall_up._x] == OBSTACLE && m_tile[corner_up._y][corner_up._x] != OBSTACLE && CheckWalkable(corner_up))
        dir_sum += (DIRECTION_INIT << corner_up_dir);

    if (m_tile[wall_down._y][wall_down._x] == OBSTACLE && m_tile[corner_down._y][corner_down._x] != OBSTACLE && CheckWalkable(corner_down))
        dir_sum += (DIRECTION_INIT << corner_down_dir);


    if (dir_sum > DIRECTION_NONE)
    {
        if (m_tile[next_point._y][next_point._x] != OBSTACLE)
            dir_sum += (DIRECTION_INIT << direction);

        //���¸���Ʈ�� ���� ��� g���� �������ֱ�
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

        //Ŭ�����Ʈ�� �̹� ���� ��� �ߴ�
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



        //���ο� ��带 ����
        JPS_Node* new_node = new JPS_Node;
        new_node->_pParent = startNode;
        new_node->_point = nowPoint;
        new_node->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, new_node->_point);
        new_node->_destDistance = CalcManhattanDist(new_node->_point, m_endPoint);
        new_node->_sumDistance = new_node->_startDistance + new_node->_destDistance;
        new_node->_directions = dir_sum;
        new_node->_num = g_num++;

        //������ ��带 ���� ��Ͽ� �ִ´�.
        m_openList.insert(make_pair(new_node->_sumDistance, new_node));


        
    }
    else 
    {
        //Ÿ�Ͽ� �湮�ߴٴ� �Ӽ� üũ�� ���ش�.
        if (m_tile[yPos][xPos] == EMPTY)
            m_tile[yPos][xPos] = VISITED;
        //Ÿ�ϻ����� �����Ѵ�.
        m_tileColor[yPos][xPos] = m_nowColor;
        //next point�� ����Ѵ�.
        FindPath_Linear(startNode, next_point, direction);
    }
}

bool CJumpPointSearch::FindPath_Linear_SubSearch(Point nowPoint, BYTE direction)
{

    //���� ����Ʈ
    Point next_point;

    //���� ��
    Point wall_up;

    //�Ʒ��� ��
    Point wall_down;

    //���� �ڳ�
    Point corner_up;

    //�Ʒ��� �ڳ�
    Point corner_down;

    //���� �ڳ� ����
    BYTE corner_up_dir;

    //�Ʒ��� �ڳ� ����
    BYTE corner_down_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //�� ���� �ջ�
    BYTE dir_sum = DIRECTION_NONE;


    switch (direction)
    {
    case UU:
        //���� ��
        wall_up = NewPoint_RR(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_RU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_LL(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_LU(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_UU(nowPoint);

        //�ڳ� �𷺼�
        corner_up_dir = RU;
        corner_down_dir = LU;

        break;

    case RR:
        //���� ��
        wall_up = NewPoint_UU(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_RU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_DD(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_RR(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = RU;
        corner_down_dir = RD;

        break;
    case DD:
        //���� ��
        wall_up = NewPoint_LL(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_LD(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_RR(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_DD(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = LD;
        corner_down_dir = RD;

        break;
    case LL:
        //���� ��
        wall_up = NewPoint_UU(nowPoint);

        //���� �ڳ�
        corner_up = NewPoint_LU(nowPoint);

        //�Ʒ��� ��
        wall_down = NewPoint_DD(nowPoint);

        //�Ʒ��� �ڳ�
        corner_down = NewPoint_LD(nowPoint);

        //���� ����Ʈ
        next_point = NewPoint_LL(nowPoint);


        //�ڳ� �𷺼�
        corner_up_dir = LU;
        corner_down_dir = LD;

        break;
    default:
        CRASH();
        break;
    }

    //�� �ִ� ������ �ʰ����� �� �ߴ�
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return false;

    //���� ������ �ߴ�
    if (m_tile[yPos][xPos] == OBSTACLE)
        return false;

    //endpoint�� �������� ���
    //�� Ž���� �Ѹ� ��ġ�� ��� ����
    if (nowPoint == m_endPoint)
        return true;


    //�ڳ� üũ
    //1. ��� ���� ���Ʒ��� �����ִ��� üũ
    //2. ��� �밢������ �������� ������ üũ
    //����Ž���� �� ������ �ʰ����� �� �ߴ�
    if (m_tile[wall_up._y][wall_up._x] == OBSTACLE && m_tile[corner_up._y][corner_up._x] != OBSTACLE && CheckWalkable(corner_up))
        return true;

    if (m_tile[wall_down._y][wall_down._x] == OBSTACLE && m_tile[corner_down._y][corner_down._x] != OBSTACLE && CheckWalkable(corner_down))
        return true;


    //Ÿ�Ͽ� �湮�ߴٴ� �Ӽ� üũ�� ���ش�.
    m_tile[yPos][xPos] = VISITED;
    //Ÿ�ϻ����� �����Ѵ�.
    m_tileColor[yPos][xPos] = m_nowColor;
    //next point�� ����Ѵ�.
    FindPath_Linear_SubSearch(next_point, direction);
    


}

void CJumpPointSearch::FindPath_Diagonal(JPS_Node* startNode, Point nowPoint, BYTE direction)
{

    //���� ����Ʈ_3��
    Point next_point_3;
 
    //���� ��
    Point wall_up_1;
    
    //�Ʒ��� ��
    Point wall_down_5;

    //���� �ڳ�_1��
    Point corner_1;

    //�Ʒ��� �ڳ�_5��
    Point corner_5;

    //�߰� ����_2��
    Point space_2;

    //�߰� ����_4��
    Point space_4;

    //1��~5�� ����
    BYTE corner_1_dir;
    BYTE corner_2_dir;
    BYTE corner_3_dir_main;
    BYTE corner_4_dir;
    BYTE corner_5_dir;

    long xPos = nowPoint._x;
    long yPos = nowPoint._y;

    //�� ���� �ջ�
    BYTE dir_sum = DIRECTION_NONE;

    switch (direction)
    {
    case RU:
        //���� ��
        wall_up_1 = NewPoint_LL(nowPoint);

        //�Ʒ��� ��
        wall_down_5 = NewPoint_DD(nowPoint);

        //���� �ڳ�
        corner_1 = NewPoint_LU(nowPoint);

        //�Ʒ��� �ڳ�
        corner_5 = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point_3 = NewPoint_RU(nowPoint);

        //����� 2��
        space_2 = NewPoint_UU(nowPoint);

        //����� 4��
        space_4 = NewPoint_RR(nowPoint);

        //�ڳ� �𷺼�
        corner_1_dir = LU;
        corner_2_dir = UU;
        corner_3_dir_main = RU;
        corner_4_dir = RR;
        corner_5_dir = RD;


        break;

    case RD:
        //���� ��
        wall_up_1 = NewPoint_UU(nowPoint);

        //�Ʒ��� ��
        wall_down_5 = NewPoint_LL(nowPoint);

        //���� �ڳ�
        corner_1 = NewPoint_RU(nowPoint);

        //�Ʒ��� �ڳ�
        corner_5 = NewPoint_LD(nowPoint);

        //���� ����Ʈ
        next_point_3 = NewPoint_RD(nowPoint);

        //����� 2��
        space_2 = NewPoint_RR(nowPoint);

        //����� 4��
        space_4 = NewPoint_DD(nowPoint);

        //�ڳ� �𷺼�
        corner_1_dir = RU;
        corner_2_dir = RR;
        corner_3_dir_main = RD;
        corner_4_dir = DD;
        corner_5_dir = LD;
        break;

    case LD:

        //���� ��
        wall_up_1 = NewPoint_UU(nowPoint);

        //�Ʒ��� ��
        wall_down_5 = NewPoint_RR(nowPoint);

        //���� �ڳ�
        corner_1 = NewPoint_LU(nowPoint);

        //�Ʒ��� �ڳ�
        corner_5 = NewPoint_RD(nowPoint);

        //���� ����Ʈ
        next_point_3 = NewPoint_LD(nowPoint);

        //����� 2��
        space_2 = NewPoint_LL(nowPoint);

        //����� 4��
        space_4 = NewPoint_DD(nowPoint);
        //�ڳ� �𷺼�
        corner_1_dir = LU;
        corner_2_dir = LL;
        corner_3_dir_main = LD;
        corner_4_dir = DD;
        corner_5_dir = RD;
        break;

    case LU:

        //���� ��
        wall_up_1 = NewPoint_RR(nowPoint);

        //�Ʒ��� ��
        wall_down_5 = NewPoint_DD(nowPoint);

        //���� �ڳ�
        corner_1 = NewPoint_RU(nowPoint);

        //�Ʒ��� �ڳ�
        corner_5 = NewPoint_LD(nowPoint);

        //���� ����Ʈ
        next_point_3 = NewPoint_LU(nowPoint);

        //����� 2��
        space_2 = NewPoint_UU(nowPoint);

        //����� 4��
        space_4 = NewPoint_LL(nowPoint);

        //�ڳ� �𷺼�
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

    //�� �ִ� ������ �ʰ����� �� �ߴ�
    if (xPos < 0 || yPos < 0 || xPos >= GRID_WIDTH || yPos >= GRID_HEIGHT)
        return;

    //���� ������ �ߴ�
    if (m_tile[yPos][xPos] == OBSTACLE)
        return;

 
    //endpoint�� �������� ���
    //endNode�� �����ϰ�
    //���� ��Ͽ� endNode�� ���� �� �ߴ�
    if (nowPoint == m_endPoint)
    {
        m_endNode->_point = m_endPoint;
        m_endNode->_pParent = startNode;
        m_endNode->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, nowPoint);
        m_endNode->_destDistance = 0;
        m_endNode->_sumDistance = m_endNode->_startDistance;
        m_endNode->_num = g_num++;

        //���� ��Ͽ� �ִ´�.
        m_openList.insert(make_pair(m_endNode->_sumDistance, m_endNode));
        return;
    }

    //�ڳ� üũ
    //1. ������� ���� �� 2���� �����ִ��� üũ
    //2. ��� ������ �������� ������ üũ
    //3. �ڳʿ� ���� ������� ���̰� �������� ������ üũ(����Ž��)

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

        //���¸���Ʈ�� ���� ��� g���� �������ֱ�
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

        //Ŭ�����Ʈ�� �̹� ���� ��� �ߴ�
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


        
        //���ο� ��带 ����
        JPS_Node* new_node = new JPS_Node;
        new_node->_pParent = startNode;
        new_node->_point = nowPoint;
        new_node->_startDistance = startNode->_startDistance + CalcPitagorasDist(startNode->_point, new_node->_point);
        new_node->_destDistance = CalcManhattanDist(new_node->_point, m_endPoint);
        new_node->_sumDistance = new_node->_startDistance + new_node->_destDistance;
        new_node->_directions = dir_sum;
        new_node->_num = g_num++;

        //������ ��带 ���� ��Ͽ� �ִ´�.
        m_openList.insert(make_pair(new_node->_sumDistance, new_node));

  
    }
    else
    {
        //Ÿ�Ͽ� �湮�ߴٴ� �Ӽ� üũ�� ���ش�.
        if(m_tile[yPos][xPos] == EMPTY)
            m_tile[yPos][xPos] = VISITED;
        //Ÿ�ϻ����� �����Ѵ�.
        m_tileColor[yPos][xPos] = m_nowColor;
        //next point�� ����Ѵ�.
        FindPath_Diagonal(startNode, next_point_3, direction);
    }

}

void CJumpPointSearch::Clear()
{
   

    //����Ʈ �ʱ�ȭ
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

    //����, ���� ��� �����
    m_startNode = new JPS_Node;
    m_endNode = new JPS_Node;

    //���� ��� �ʱ�ȭ
    m_startNode->_destDistance = 0;
    m_startNode->_point._x = 0;
    m_startNode->_point._y = 0;
    m_startNode->_sumDistance = 0;

    //���� ��� �ʱ�ȭ
    m_startNode->_destDistance = 0;
    m_startNode->_point._x = 0;
    m_startNode->_point._y = 0;
    m_startNode->_sumDistance = 0;

    //���� ����Ʈ �ʱ�ȭ
    m_endPoint._x = 0;
    m_endPoint._y = 0;

    //2���� �迭 �ʱ�ȭ
    ZeroMemory(m_tile, GRID_HEIGHT * GRID_WIDTH);
    ZeroMemory(m_tileColor, GRID_HEIGHT * GRID_WIDTH * sizeof(Color));

    //��ȣ �ʱ�ȭ
    g_num = 0;
}

void CJumpPointSearch::RandomColor()
{
    //�� �缳��
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
