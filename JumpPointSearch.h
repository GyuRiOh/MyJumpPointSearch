#pragma once

#include <Windows.h>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>

using namespace std;

#define CRASH() do{\
int* a = nullptr;\
*a = 100;\
} while(0)

#define DIRECTION_NONE 0
#define DIRECTION_INIT 1
#define DIRECTION_UU 1
#define DIRECTION_RU DIRECTION_UU << 1
#define DIRECTION_RR DIRECTION_UU << 2
#define DIRECTION_RD DIRECTION_UU << 3
#define DIRECTION_DD DIRECTION_UU << 4
#define DIRECTION_LD DIRECTION_UU << 5
#define DIRECTION_LL DIRECTION_UU << 6
#define DIRECTION_LU DIRECTION_UU << 7
#define DIRECTION_ALL 0xff

constexpr int GRID_WIDTH = 70;
constexpr int GRID_HEIGHT = 30;

enum property
{
    EMPTY = 0,
    OBSTACLE,
    START,
    END,
    VISITED,
    NODE,
    REVISITED
};

enum direction
{
    UU = 0,
    RU,
    RR,
    RD,
    DD,
    LD,
    LL,
    LU
};

struct Point
{
    long _x;
    long _y;

    Point()
    {

    }

    Point(int x, int y)
    {
        _x = x;
        _y = y;
    }

    bool operator== (const Point& a) const {
        if (_x == a._x && _y == a._y)
            return true;
        else
            return false;
    }

    bool operator!= (const Point& a) const {
        if (_x == a._x && _y == a._y)
            return false;
        else
            return true;
    }
    
};


struct Color
{
    BYTE _red;
    BYTE _green;
    BYTE _blue;

    Color()
    {

    }

    Color(BYTE red, BYTE green, BYTE blue)
    {
        _red = red;
        _green = green;
        _blue = blue;
    }

};

typedef struct JPS_Node
{
    Point _point;
    float _destDistance;
    float _startDistance;
    float _sumDistance;

    JPS_Node* _pParent;
    BYTE _directions;

    DWORD _num;

    JPS_Node()
    {
        _point._x = 0;
        _point._y = 0;
        _destDistance = 0;
        _startDistance = 0;
        _sumDistance = 0;
        _pParent = nullptr;
        _directions = DIRECTION_NONE;
    }

}jps_Node;

class CJumpPointSearch
{
public:
    CJumpPointSearch();
    ~CJumpPointSearch();

    void Initiate();
    void FindPath_Loop();
    void FindPath_Linear(JPS_Node* startNode, Point nowPoint, BYTE direction);
    bool FindPath_Linear_SubSearch(Point nowPoint, BYTE direction);
    void FindPath_Diagonal(JPS_Node* startNode, Point nowPoint, BYTE direction);

    void Clear();

    //Ÿ���� �����ϰ� ����
    void RandomTile();


private:
    //=========================================
    // �ܼ� ���
    //==========================================
    inline int CalcManhattanDist(Point start, Point dest)
    {
        int result = 0;

        result += abs(dest._x - start._x);
        result += abs(dest._y - dest._y);

        return result;
    }

    inline int CalcPitagorasDist(Point start, Point dest)
    {

        int result = sqrt(pow(start._x - dest._x, 2) + pow(start._y - dest._y, 2));
        return result;
    }

    inline Point NewPoint_UU(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x;
        newPoint._y = curPoint._y - 1;

        return newPoint;
    }

    inline Point NewPoint_RU(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x + 1;
        newPoint._y = curPoint._y - 1;

        return newPoint;
    }

    inline Point NewPoint_RR(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x + 1;
        newPoint._y = curPoint._y;

        return newPoint;
    }

    inline Point NewPoint_RD(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x + 1;
        newPoint._y = curPoint._y + 1;

        return newPoint;
    }

    inline Point NewPoint_DD(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x;
        newPoint._y = curPoint._y + 1;

        return newPoint;
    }

    inline Point NewPoint_LD(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x - 1;
        newPoint._y = curPoint._y + 1;

        return newPoint;
    }


    inline Point NewPoint_LL(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x - 1;
        newPoint._y = curPoint._y;

        return newPoint;
    }

    inline Point NewPoint_LU(Point curPoint)
    {
        Point newPoint;

        newPoint._x = curPoint._x - 1;
        newPoint._y = curPoint._y - 1;

        return newPoint;
    }

    void RandomColor();
    bool CheckWalkable(Point point);
    void PrintNodeInTile();
    bool Brasenham(Point srcPoint, Point dstPoint);
    void ReFindPath();

public:
    //Ÿ���� ǥ���ϴ� 2���� �迭
    char m_tile[GRID_HEIGHT][GRID_WIDTH];
    //0 ��ֹ� ����/ 1 ��ֹ� ����

    //Ÿ�� ������ ǥ���ϴ� 2���� �迭
    Color m_tileColor[GRID_HEIGHT][GRID_WIDTH];

    //���ڸ� ǥ���ϴ� 2���� �迭
    char m_tileNumber[GRID_HEIGHT][GRID_WIDTH];

    //����, ���� ���
    JPS_Node* m_startNode;
    JPS_Node* m_endNode;

    //���� ��ǥ
    Point m_endPoint;

    //ã���� ��ã�� ���θ� �÷��׷� üũ.
    bool foundFlag = true;

    //���� ����Ʈ
    //Key : F��, Value : ��� ������
    multimap<float, JPS_Node*> m_openList;

    //Ŭ���� ����Ʈ
    //Key : ����NUM, Value : ��� ������
    map<DWORD, JPS_Node*> m_closeList;

private:
    //�ڳ�üũ�ϴ� �Ÿ��� Ƚ�� ���� �ɱ�
    int iMaxJump;

    //������ ���� �÷�
    Color m_nowColor;


};