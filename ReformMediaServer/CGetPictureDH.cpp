#include "CGetPicture.h"
#include "ReformMediaServerDef.H"
#include "Log.h"
#include <Windows.h>
#include "dhplay.h"

CGetIconDH* CGetIconDH::_instance=NULL;

CGetIconDH::CGetIconDH()
	:_init(false)
{
}

bool CGetIconDH::init()
{
	bool ret = false;
	if(!_init)
	{
		if(!PLAY_GetFreePort(&icon_port))
		{
		}
		else
		{
			_init = true;
			ret=true;
		}
	}
	else
		ret = true;
	return ret;
}

int CGetIconDH::getIcon(char**buf, const char* file) const
{
	DWORD size = 0;
	if(!PLAY_OpenFile(icon_port,(LPSTR)file))
	{
	}
	else if(!PLAY_Play(icon_port,NULL))
	{
	}
	else
	{
		int i = 0;
		while(i++ < 150)
		{
			DWORD fnum = PLAY_GetCurrentFrameNum(icon_port);
			if(fnum > 0)
			{
				LONG w,h;
				PLAY_GetPictureSize(icon_port, &w, &h);
				DWORD buf_size = w*h*3/2;
				*buf = new char[buf_size];
				while(!PLAY_GetPicJPEG(icon_port,(PBYTE)(*buf),buf_size,&size))
				{
					g_pLog->WriteLog("×¥Í¼Ê§°Ü,%lu\n", PLAY_GetLastError(icon_port));
				}
				fnum = PLAY_GetCurrentFrameNum(icon_port);
				break;
			}
			Sleep(50);
		}
		PLAY_Stop(icon_port);
		PLAY_CloseFile(icon_port);
	}
	return size;
}
