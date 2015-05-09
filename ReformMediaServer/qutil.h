#ifndef __QUTIL_H__
#define __QUTIL_H__

class CQCritSec {

    // make copy constructor and assignment operator inaccessible

    CQCritSec(const CQCritSec &refCritSec);
    CQCritSec &operator=(const CQCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;
public:
    CQCritSec() {
	InitializeCriticalSection(&m_CritSec);
    };

    ~CQCritSec() {
	DeleteCriticalSection(&m_CritSec);
    };

    void Lock() {
	EnterCriticalSection(&m_CritSec);
    };

    void Unlock() {
	LeaveCriticalSection(&m_CritSec);
    };
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CQAutoLock {

    // make copy constructor and assignment operator inaccessible

    CQAutoLock(const CQAutoLock &refAutoLock);
    CQAutoLock &operator=(const CQAutoLock &refAutoLock);

protected:
    CQCritSec * m_pLock;

public:
    CQAutoLock(CQCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CQAutoLock() {
        m_pLock->Unlock();
    };
};

#endif