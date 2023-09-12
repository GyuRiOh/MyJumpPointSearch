/*---------------------------------------------------------------

	procademy MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);

				
----------------------------------------------------------------*/
#ifndef  __PROCADEMY_MEMORY_POOL__
#define  __PROCADEMY_MEMORY_POOL__
#include <new.h>


#define CRASH() do{\
int* a = nullptr;\
*a = 100;\
} while(0)

namespace server_baby
{



	template <class DATA>
	class CMemoryPool
	{



	public:
#pragma pack(1)
		struct st_BLOCK_NODE
		{
			unsigned short code;
			unsigned long long int index;
			unsigned int overguard;
			DATA data;
			unsigned int underguard;
			st_BLOCK_NODE* next;
			bool is_called_Destructor = false;
		};

#pragma pack(pop)

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, unsigned short shCode, bool bPlacementNew = false)
		{	


			m_dummy_top_Node = new st_BLOCK_NODE;
			delete_list = (st_BLOCK_NODE**)malloc(sizeof(st_BLOCK_NODE*) * iBlockNum);

			st_BLOCK_NODE* pNext = m_dummy_top_Node;
			for (int i = 0; i < iBlockNum; i++)
			{
				st_BLOCK_NODE* pBlock = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
				delete_list[i] = pBlock;
				pBlock->code = shCode;
				pBlock->overguard = m_overguard;
				pBlock->underguard = m_underguard;
				pBlock->next = nullptr;
				pNext->next = pBlock;
				pNext = pBlock;

				//생성자 호출
				if (bPlacementNew)
					new (&(pBlock->data)) DATA;
			}

			_pFreeNode = m_dummy_top_Node->next;
			m_capacity = iBlockNum;
			m_useCount = 0;
			m_placementNew = bPlacementNew;			
			m_indexStart = m_indexNow = shCode * iBlockNum;
			m_code = shCode;
		}

		//이걸 어떻게 한담?
		virtual	~CMemoryPool()
		{
			if (m_placementNew)
			{
				for (int i = 0; i < m_capacity; i++)
				{
					//소멸자가 이미 호출되버린 경우 delete를 어쩐다..?
					if (delete_list[i]->is_called_Destructor == false)
					{
						DATA* pDATA = &(delete_list[i]->data);
						pDATA->~DATA();
					}

					free(delete_list[i]);

				}
			}
			else
			{
				for (int i = 0; i < m_capacity; i++)
				{

					free(delete_list[i]);

				}
			}

			delete m_dummy_top_Node;
		}

		//////////////////////////////////////////////////////////////////////////
		// 블럭 하나를 할당받는다.  
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		 DATA* Alloc(void)
		{			

			st_BLOCK_NODE* pAlloc = _pFreeNode;
			//인덱스 삽입
			pAlloc->index = m_indexNow++;


			_pFreeNode = _pFreeNode->next;
			m_useCount++;
			return &(pAlloc->data);
		}

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		 bool Free(DATA* pData)
		{		

			//오버, 언더플로우 여부 체크	
			if (*((unsigned int*)pData - 1) != m_overguard || *(unsigned int*)(pData + 1) != m_underguard)
				CRASH();

			//코드 확인
			void* pCode = (char*)pData - 
				(sizeof(unsigned short) + 
				sizeof(unsigned long long int) + 
				sizeof(unsigned int));

			if (*(unsigned short*)pCode != m_code)
				CRASH();

			//소멸자 호출
			if (m_placementNew)
			{
				pData->~DATA();
				bool* pDestructor = (bool*)pData + (sizeof(DATA) + sizeof(unsigned int) + sizeof(st_BLOCK_NODE*));
				*pDestructor = true;
			}

			//포인터 끊고 연결
			void* pNext = (char*)pData + (sizeof(DATA) + sizeof(unsigned int));
			memmove(pNext, &_pFreeNode, sizeof(st_BLOCK_NODE*));
			_pFreeNode = (st_BLOCK_NODE*)pCode;

			m_useCount--;

			return true;

		}

		//////////////////////////////////////////////////////////////////////////
		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		//
		// Parameters: 없음.
		// Return: (int) 메모리 풀 내부 전체 개수
		//////////////////////////////////////////////////////////////////////////
		inline int	GetCapacityCount(void) 
		{ 
			return m_capacity; 	
		}

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 블럭 개수를 얻는다.
		//
		// Parameters: 없음.
		// Return: (int) 사용중인 블럭 개수.
		//////////////////////////////////////////////////////////////////////////
		inline int	GetUseCount(void)
		{ 
			return m_useCount; 
		}


	private:
		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
		st_BLOCK_NODE *_pFreeNode;

		//delete용 배열
		st_BLOCK_NODE** delete_list;

		// 더미 노드
		st_BLOCK_NODE* m_dummy_top_Node;

		//현재 확보된 블럭 개수(메모리풀 내부의 전체 개수)
		int m_capacity;

		//사용중인 블럭 개수
		int m_useCount;

		//생성자 호출 여부
		bool m_placementNew;

		//인덱스 시작
		unsigned long long int m_indexStart;

		//현재 할당된 인덱스
		unsigned long long int m_indexNow;

		//메모리가드
		unsigned int m_overguard = 0xcdcdcdcd;
		unsigned int m_underguard = 0xfdfdfdfd;

		//고유 코드
		unsigned short m_code;

	};

	//상황에 따라 별도로 힙을 만들 수 있다.


}



















#endif