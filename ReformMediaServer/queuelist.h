#ifndef __CQ_LIST_H__
#define __CQ_LIST_H__

#include "qutil.h"
#include <list>
#include <map>

#define QE_OK				0
#define QE_NOERROR			0
#define QE_FAIL				-1
#define QE_FORCE_EXIT		-2

//--------- common error [ -10 -- -99 ] -----------------
#define QE_PARAM_FAULT		-10	//参数错误
#define QE_UNINIT			-11
#define QE_INIT_MORE		-12	//初始化多次
#define QE_TIME_OUT			-13
#define QE_WAIT_FAILED		-14

using namespace std;

class CQBuffParam
{
public:
    virtual int GetMaxCount() = 0;
    virtual char *GetName() = 0;
    virtual void GetParam(int &nCount,int &nPlus,int &nMinus) = 0;
};

template <class T>
class CQBuffBase : public CQBuffParam
{
public:
	enum {
		MAX_BUFF_COUNT = 300000,	// Maximum count for the semaphore object
		MAX_LIST_COUNT = 5000		// 最大的队列数
	};
	//CQBuffBase(const CQBuffBase &refBase);
	//CQBuffBase &operator=(const CQBuffBase &refBase);
protected:
	CQCritSec m_csLock;

	HANDLE m_hSem;
	HANDLE m_hExitEvent;

	BOOL   m_bInit;
    int    m_nMaxCount;
    char   m_szName[128];
public:
	CQBuffBase()
	 : m_hSem(NULL),m_hExitEvent(NULL),m_bInit(FALSE)
	{
        m_nMaxCount = 0;
        m_szName[0] = 0;
	}

	virtual ~CQBuffBase()
	{
		UnInit();
	}

    virtual int GetMaxCount()
    {
        return m_nMaxCount;
    }

    virtual char *GetName()
    {
        return m_szName;
    }

	virtual BOOL GetStatus()
	{
		return m_bInit;
	}

protected:
	virtual void UnInit()
	{
		if(m_hSem)
		{
			CloseHandle(m_hSem);
			m_hSem = NULL;
		}
	}

public:
	virtual BOOL Init(const char *pszName, HANDLE hExit = NULL, int nMaxCount = MAX_BUFF_COUNT)
	{
        strcpy_s(m_szName, pszName);
        m_nMaxCount = nMaxCount;
		m_hExitEvent = hExit;
		m_hSem = CreateSemaphore( NULL,						// SD
								  0,                        // initial count
								  m_nMaxCount,				// maximum count
								  NULL                      // object name
								);
		if (m_hSem == NULL)
		{
			m_bInit = FALSE;
			return FALSE;
		}
		m_bInit = TRUE;
		return TRUE;
	}

	HANDLE GetSemphore() const { return m_hSem; } 
	
	/*===============================================================*/
	/*  往队列中插入一个项
	/*----------------------------------------------------------------
	/*  ReturnV:
	/*			0	---------	插入成功
	/*			-1	---------	插入出错
	/*			1	---------	插入成功，但是由于队列已满，故先清除所有的，再插入
	/*===============================================================*/
	virtual int  PutBack(const T& x) = 0;
	virtual int  GetHead(T *p) = 0;
	virtual int  GetHeadAndWait(T *p,int nTimeOut) = 0;	
};

template <class T> 
class CQBuffList : public CQBuffBase<T>
{	
	typedef list<T> QLIST;

public:
	QLIST	m_List;

    int     m_nPlus;
    int     m_nMinus;

protected:
	virtual void UnInit()
	{
		m_List.clear();
	}

public:
	CQBuffList()
    {
        m_nPlus = m_nMinus = 0;
    };

	~CQBuffList()
	{
		UnInit();
	}

    int GetCount()
    {
        CQAutoLock lock(&m_csLock);
        return m_List.size();
    }

    void CleanData()
    {
        CQAutoLock lock(&m_csLock);
        m_List.clear();
    }

    void CleanData(int nCount)
    {
        CQAutoLock lock(&m_csLock);
        int i = 0;
        while (i<nCount && m_List.size()>0)
        {
            m_List.pop_front();
            i++;
        }
    }

    virtual void GetParam(int &nCount,int &nPlus,int &nMinus)
    {
        CQAutoLock lock(&m_csLock);
        nCount = (int)m_List.size();
        nPlus = m_nPlus;
        nMinus = m_nMinus;
        m_nPlus = m_nMinus = 0;
    }

	virtual int PutBack(const T& x)
	{
		//ASSERT(m_bInit);

		CQAutoLock lock(&m_csLock);
		if (m_List.size() > MAX_LIST_COUNT)
		{
			ReleaseSemaphore(m_hSem,1,NULL);
			return 1;
		}
        m_nPlus++;
		m_List.push_back(x);
		ReleaseSemaphore(m_hSem,1,NULL);
		return 0;
	}

	virtual int GetHead(T *p)
	{
		//ASSERT(m_bInit);

		CQAutoLock lock(&m_csLock);

		if(m_List.empty())
			return -1;

        m_nMinus++;
		*p = m_List.front();
		m_List.pop_front();

		return 0;
	}

	virtual int GetHeadAndWait(T *p,int nTimeOut);
};

template <class T>
int  CQBuffList<T>::GetHeadAndWait(T *p,int nTimeOut)
{
	//ASSERT(m_bInit);
	//ASSERT(m_hSem  != NULL);
	//ASSERT(m_hExitEvent != NULL);

	HANDLE hev[] = {m_hExitEvent, m_hSem};
	DWORD dwRet;

	dwRet = WaitForMultipleObjects(2, hev, FALSE, nTimeOut * 1000);
	switch(dwRet)
	{
		case WAIT_OBJECT_0 + 1:
			{
				CQAutoLock lock(&m_csLock);
				*p = m_List.front();			
				m_List.pop_front();
					
				return QE_OK;
			}
		case WAIT_OBJECT_0 :
			return QE_FORCE_EXIT;
		case WAIT_TIMEOUT:
			return QE_TIME_OUT;
		default:
			return QE_WAIT_FAILED;
	}
};

/*========================================================================================*/
/*			CQSimpleList
/*========================================================================================*/
template <class T>
class CQSimpleList
{
    typedef list<T> QLIST;
	QLIST	m_List;
    CQCritSec m_csLock;
protected:
	virtual void UnInit()
	{
		m_List.clear();
	}
public:
	CQSimpleList()
    {

    };
	~CQSimpleList()
	{
		UnInit();
	}

    int GetCount()
    {
        CQAutoLock lock(&m_csLock);
        return m_List.size();
    }

	virtual int PutBack(const T& x)
	{
		CQAutoLock lock(&m_csLock);

		if (m_List.size() > MAX_LIST_COUNT)
			return 1;
		m_List.push_back(x);
		return 0;
	}

	virtual int  GetHead(T *p)
	{
		CQAutoLock lock(&m_csLock);

		if(m_List.empty())
			return -1;

		*p = m_List.front();
		m_List.pop_front();
        return 0;
	}
};

/*========================================================================================*/
/*			CQLargeBuffList
/*========================================================================================*/
template <class T>
class CQLargeBuffList : public CQBuffBase<T>
{
	T *  m_pArray;
	enum { PACK_SIZE = sizeof(T) };
	UINT		m_nBufferCount;
	UINT    	m_nHeadIndex;   /////// the first item with empty
	UINT		m_nTailIndex;   /////// the first item with full
	UINT		m_nUsedCount;
	
	BOOL IsFull() 
	{
		return (m_nUsedCount == m_nBufferCount);
	}

	BOOL IsEmpty() 
	{
		//CQAutoLock lock(&m_csLock);
		return (m_nUsedCount==0 );
	}

public:
	CQLargeBuffList()
	{
		m_nBufferCount = 0;
		m_nHeadIndex = 0;
		m_nTailIndex = 0;
		m_nUsedCount = 0;
		m_pArray = NULL;
	};
	~CQLargeBuffList() 
	{
		ClearBuffer();
	}

	void ClearBuffer()
	{
		//确保 GetHeadAndWait() 已经退出
		SetEvent(m_hExitEvent);		
		CQAutoLock lock(&m_csLock);
		if(m_pArray)		
		{
			delete [] m_pArray;
			m_pArray = NULL;
		}
		m_nUsedCount = 0;
		m_nHeadIndex = 0;
		m_nTailIndex = 0;

		m_nBufferCount = 0;
		m_bInit = FALSE;
	}

	void CleanPacket() 
	{
		while(1)
		{
			if(WaitForSingleObject(m_hSem, 0) != WAIT_OBJECT_0 )
				break;
			//m_nUsedCount--;
		}	
		CQAutoLock lock(&m_csLock);
		m_nUsedCount = 0;
		m_nHeadIndex = 0;
		m_nTailIndex = 0;
	}

	BOOL InitParam(int nCount,HANDLE hExit) 
	{
		ASSERT(nCount > 0 );	
		ASSERT(hExit != NULL);
		if(m_pArray)
			return FALSE;
		
		m_pArray = new T[nCount];
		if(m_pArray == NULL)
			return FALSE;
		m_nBufferCount = nCount;	
		
		m_nHeadIndex = 0;
		m_nTailIndex = 0;
		m_nUsedCount = 0;
		return CQBuffBase<T>::Init(hExit);
	}
	
	/*===============================================================*/
	/*  往队列中插入一个项
	/*----------------------------------------------------------------
	/*  ReturnV:
	/*			0	---------	插入成功
	/*			-1	---------	未初始化
	/*			1	---------	插入成功，但是由于队列已满，故先清除所有的，再插入
	/*			-2  ---------   内存出错
	/*			-3  ---------   已经退出
	/*===============================================================*/
	virtual int PutBack(const T& x) 
	{
		if(!m_bInit)
			return -1;
		if(WaitForSingleObject(m_hExitEvent,0) == WAIT_OBJECT_0 )
		{
			return -3;
		}

		int nRet = 0;
		CQAutoLock lock(&m_csLock);
		if(IsFull())
		{
			while(1)
			{
				if(WaitForSingleObject(m_hSem,0) != WAIT_OBJECT_0 )
					break;				
			}	
			m_nUsedCount = 0;
			m_nHeadIndex = 0;
			m_nTailIndex = 0;
			nRet = 1;
		}		
		
		if(!m_pArray)
		{
			return -2;
		}
		memcpy(&m_pArray[m_nHeadIndex],&x,PACK_SIZE);
		m_nHeadIndex = ++m_nHeadIndex % m_nBufferCount;
		m_nUsedCount++;
		ReleaseSemaphore(m_hSem,1,NULL);
		return nRet;		
	}
	
	virtual int  GetHead(T *p) 
	{	
		if(!m_bInit || p == NULL)
			return -1;
		CQAutoLock lock(&m_csLock);
		if(IsEmpty())
			return -1;
		memmove(p,&m_pArray[m_nTailIndex],PACK_SIZE);
		m_nTailIndex = ++m_nTailIndex % m_nBufferCount;
		m_nUsedCount--;
		return 0;
	}
	
	virtual int  GetHeadAndWait(T *p,int nTimeOut) 
	{
		if(!m_bInit || p == NULL || nTimeOut < 0 )
			return QE_PARAM_FAULT;
		
		ASSERT(m_hSem != NULL);
		ASSERT(m_hExitEvent != NULL);
		HANDLE hev[] = {m_hExitEvent,m_hSem};
		DWORD dwRet;
		dwRet = WaitForMultipleObjects(2, hev, FALSE, nTimeOut*1000);
		switch(dwRet)
		{
			case WAIT_OBJECT_0 + 1:
			{
				CQAutoLock lock(&m_csLock);
				if(!m_pArray)
				{
					return QE_FORCE_EXIT;
				}
				memmove(p,&m_pArray[m_nTailIndex],PACK_SIZE);
				m_nTailIndex = ++m_nTailIndex % m_nBufferCount;
				m_nUsedCount--;
				if(m_nUsedCount < 0)
				{
					m_nUsedCount = 0;
				}
				//g_pEventLog->DebugParamInfo(" UseCount = %d ",m_nUsedCount);
				return QE_OK;
			}
			case WAIT_OBJECT_0 :
				return QE_FORCE_EXIT;
			case WAIT_TIMEOUT:
				return QE_TIME_OUT;
			default:
				return QE_WAIT_FAILED;
		}		
	}
	
	UINT GetCount() 
	{
		CQAutoLock lock(&m_csLock);
		return m_nUsedCount;
	}	
};

/*--------------------------------------------------------------
/*  class CQHash remark:
/*		 this class implement the following functions:
/*		 1. create a hash table;
/*
/*	ARCHECTITURE:
/*	|
/*	|---|(key = 1)
/*	|	----| (1,data1)
/*	|		| (1,data2)
/*	|		| (1,data3)
/*	|
*	|---|(key = 2)
/*	|	----| (2,data4)
/*	|		| (2,data5)
/*	|		| (2,data6)
/*	|
/*
/*  NOTE:  
/*		 
/*		
/*	USE METHOD:
/*	{
/*	
/*	}
/*	函数返回值定义声明：
/*		
/*--------------------------------------------------------------*/

template <class Key,class T>
class CQHash 
{	
protected:
	struct _HashNodeInfo;

	struct _HashNode
	{
		_HashNode *pNext;
		_HashNode *pPrev;
		T Data;
		_HashNodeInfo *pInfo;
		_HashNode()
		{
			pInfo = NULL;
			pNext = pPrev = NULL;
		}
	};

	struct _HashNodeInfo
	{
		_HashNode *pNode;
		bool	bExist;
		_HashNodeInfo()
		{
			pNode = NULL;
			bExist = true;
		}
	};

	class CQNodeList
	{
		_HashNode *m_pFirst;
		//int	nCount;
	public:
		CQNodeList()
		{
			m_pFirst = NULL;
			//nCount = 0;
		}
		~CQNodeList()
		{
			_HashNode *p,*pNode = m_pFirst;
			while(pNode)
			{
				p = pNode;
				pNode = pNode->pNext;				
				delete p;
			}
		}
		void push(_HashNode *p)
		{
			if(m_pFirst)
			{
				m_pFirst->pPrev = p;
				p->pNext = m_pFirst;
				m_pFirst = p;
			}else
			{
				m_pFirst = p;
			}
		}
	};

	typedef map<Key,CQNodeList*> NODEMAP;
	NODEMAP	m_NodeMap;

	typedef list <_HashNodeInfo*> NODELIST;
	NODELIST m_NodeList;

	int		m_nNodeCount;
	int		m_nMaxCount;
protected:
	void Remove(_HashNode *pNode) 
	{
		_HashNode *p,*pFirst;
		if(pNode)
		{
			if(pNode->pPrev)
				pNode->pPrev->pNext = NULL;
			else
				pFirst = pNode; 
			while(pNode)
			{
				p = pNode;
				pNode = pNode->pNext;	
				if(p->pInfo)
					p->pInfo->bExist = false;
				delete p;
				m_nNodeCount--;
			}
			pFirst = NULL;
		}
	}

public:
	CQHash(int nMaxCount) : m_nMaxCount(nMaxCount), m_nNodeCount(0) {};

	~CQHash() 
	{
		CQNodeList* pList;
		NODEMAP::iterator pos = m_NodeMap.begin();
		while(pos != m_NodeMap.end())
		{
			pList = pos->second;
			pos++;
			delete  pList;			
		}
		
		_HashNodeInfo *pInfo;
		NODELIST::iterator pos1 = m_NodeList.begin();
		while(pos1 != m_NodeList.end())
		{
			pInfo = *pos1;
			pos1++;
			delete pInfo;			
		}
	}

	virtual void Push(const Key &k,const T &data)
	{
		_HashNode *pNode = new _HashNode();
		_HashNodeInfo *pInfo = new _HashNodeInfo();
		CQNodeList *pList;
		
		pInfo->bExist = true;
		pInfo->pNode = pNode;
		
		pNode->pInfo = pInfo;
		pNode->Data = data;
		
		NODEMAP::iterator pos = m_NodeMap.find(k);
		if( pos != m_NodeMap.end())
		{
			pList = pos->second;
			pList->push(pNode);
		}else
		{
			pList = new CQNodeList();
			pList->push(pNode);
			m_NodeMap[k] = pList;
		}
		m_NodeList.push_back(pInfo);
		m_nNodeCount++;
	}

	int GetNodeCount()
	{
		return m_nNodeCount;		
	}
};

#endif