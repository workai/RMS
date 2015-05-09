#include "CGetPicture.h"
#include "StdHeader.h"
//#include "HCNetSDK.h"
//#include "HikNetDVR.h"
#include "PlayM4.h"
#include "ReformMediaServerDef.h"
#include "Log.h"

CGetIconHK* CGetIconHK::_instance=NULL;
CGetIconFormFile* CGetIconFormFile::_instance= NULL;

CGetIconHK::CGetIconHK()
	:_init(false)
{
}

bool CGetIconHK::init()
{
	bool ret = false;
	if(!_init)
	{
		DWORD err = 0;
		BYTE pHeader[40]={73,77,75,72,1,1,0,0,2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		if(!PlayM4_GetPort(&icon_port))
		{
			g_pLog->WriteLog("抓图失败,获取端口号失败\n");
		}
		else
		{
			_init = true;
			ret = true;
		}
	}
	else
		ret = true;
	return ret;
}

int CGetIconHK::getIcon(char**buf, const char* file) const
{
	DWORD size=0;
	if(!PlayM4_OpenFile(icon_port, (LPSTR)file))
	{
	}
	else if(!PlayM4_Play(icon_port,NULL))
	{
	}
	else
	{
		int i=0;
		while(i++<150)
		{
			DWORD fnum = PlayM4_GetCurrentFrameNum(icon_port);
			if(fnum <50)
			{
				LONG w,h;
				PlayM4_GetPictureSize(icon_port, &w, &h);
				//DWORD buf_size = w*h*4 +sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
				//*buf = new char[buf_size];
				//if(!PlayM4_GetBMP(icon_port, (PBYTE)(*buf), buf_size, &size))
				//{
				//}
				DWORD buf_size = w*h*3/2;
				*buf = new char[buf_size];
				if(!PlayM4_GetJPEG(icon_port, (PBYTE)(*buf), buf_size, &size))
				{
					g_pLog->WriteLog("抓图失败,%lu\n", PlayM4_GetLastError(icon_port));
				}
				break;
			}
			Sleep(50);
		}
		PlayM4_Stop(icon_port);
		PlayM4_CloseFile(icon_port);
	}
	return size;
}

CGetIconFormFile::CGetIconFormFile()
{
}

int CGetIconFormFile::getIcon(char** buf, const char* file, const char* type) const
{
	int ret = 0;
	if(strstr(type, "海康")!=NULL)
	{
		ret = CGetIconHK::Instance()->getIcon(buf, file);
	}
	else if(strstr(type, "大华")!=NULL)
	{
		ret = CGetIconDH::Instance()->getIcon(buf, file);
	}
	return ret;
}

