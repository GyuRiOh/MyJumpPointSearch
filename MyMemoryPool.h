/*---------------------------------------------------------------

	procademy MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

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
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
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

				//������ ȣ��
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

		//�̰� ��� �Ѵ�?
		virtual	~CMemoryPool()
		{
			if (m_placementNew)
			{
				for (int i = 0; i < m_capacity; i++)
				{
					//�Ҹ��ڰ� �̹� ȣ��ǹ��� ��� delete�� ��¾��..?
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
		// �� �ϳ��� �Ҵ�޴´�.  
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		 DATA* Alloc(void)
		{			

			st_BLOCK_NODE* pAlloc = _pFreeNode;
			//�ε��� ����
			pAlloc->index = m_indexNow++;


			_pFreeNode = _pFreeNode->next;
			m_useCount++;
			return &(pAlloc->data);
		}

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		 bool Free(DATA* pData)
		{		

			//����, ����÷ο� ���� üũ	
			if (*((unsigned int*)pData - 1) != m_overguard || *(unsigned int*)(pData + 1) != m_underguard)
				CRASH();

			//�ڵ� Ȯ��
			void* pCode = (char*)pData - 
				(sizeof(unsigned short) + 
				sizeof(unsigned long long int) + 
				sizeof(unsigned int));

			if (*(unsigned short*)pCode != m_code)
				CRASH();

			//�Ҹ��� ȣ��
			if (m_placementNew)
			{
				pData->~DATA();
				bool* pDestructor = (bool*)pData + (sizeof(DATA) + sizeof(unsigned int) + sizeof(st_BLOCK_NODE*));
				*pDestructor = true;
			}

			//������ ���� ����
			void* pNext = (char*)pData + (sizeof(DATA) + sizeof(unsigned int));
			memmove(pNext, &_pFreeNode, sizeof(st_BLOCK_NODE*));
			_pFreeNode = (st_BLOCK_NODE*)pCode;

			m_useCount--;

			return true;

		}

		//////////////////////////////////////////////////////////////////////////
		// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
		//
		// Parameters: ����.
		// Return: (int) �޸� Ǯ ���� ��ü ����
		//////////////////////////////////////////////////////////////////////////
		inline int	GetCapacityCount(void) 
		{ 
			return m_capacity; 	
		}

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		inline int	GetUseCount(void)
		{ 
			return m_useCount; 
		}


	private:
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		st_BLOCK_NODE *_pFreeNode;

		//delete�� �迭
		st_BLOCK_NODE** delete_list;

		// ���� ���
		st_BLOCK_NODE* m_dummy_top_Node;

		//���� Ȯ���� �� ����(�޸�Ǯ ������ ��ü ����)
		int m_capacity;

		//������� �� ����
		int m_useCount;

		//������ ȣ�� ����
		bool m_placementNew;

		//�ε��� ����
		unsigned long long int m_indexStart;

		//���� �Ҵ�� �ε���
		unsigned long long int m_indexNow;

		//�޸𸮰���
		unsigned int m_overguard = 0xcdcdcdcd;
		unsigned int m_underguard = 0xfdfdfdfd;

		//���� �ڵ�
		unsigned short m_code;

	};

	//��Ȳ�� ���� ������ ���� ���� �� �ִ�.


}



















#endif