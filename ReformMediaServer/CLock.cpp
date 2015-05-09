#include "CLock.h"
#include <Windows.h>

CLock::CLock()
	:_event(CreateEvent(NULL,FALSE,TRUE,NULL)), _status(0)
{
}

CLock::~CLock()
{
	CloseHandle(_event);
}

int CLock::lock()
{
	unsigned int s = InterlockedCompareExchange(&_status, 1, 0);
	if(s != 0)
	{
		while(InterlockedCompareExchange(&_status,2,0) != 0)
		{
			DWORD ret= WaitForSingleObject(_event, INFINITE);
			if(ret != WAIT_OBJECT_0)
			{
				//log
				return FALSE;
			}
		}
	}
	return TRUE;
}

int CLock::unlock()
{
	unsigned int s = InterlockedExchange(&_status, 0);
	if(!SetEvent(_event))
	{
		DWORD err = GetLastError();
	}
	return TRUE;
}

int CTry::have_a_try()
{
	return InterlockedExchange(&_status, 0);
}

void CTry::finish()
{
	InterlockedExchange(&_status, 1);
}
