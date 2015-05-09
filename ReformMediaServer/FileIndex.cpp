#include "StdHeader.h"
#include "FileIndex.h"
#include "SysInfo.h"
#include "Packet.h"

CFileIndex::CFileIndex(void)
{
}

CFileIndex::~CFileIndex(void)
{
}

// 修改
BOOL CFileIndex::Init(_int64 starttimepos, _int64 endtimepos, char *channelid)
{
	m_FileArray.RemoveAll();
	char strpath[MAX_PATH]={};

	//sprintf(strpath, "");
	
//	sprintf(strpath, "%s%d", g_pSysInfo->m_FileRecordPath, channelid);

   CPacket inPacket;
   DOMElement* AccNode = NULL;
   DOMElement* AccNode1 = NULL;
   DOMElement* AccNode2 = NULL;

   if(inPacket.BuiltTree(g_pSysInfo->m_XMLFilePath) == -1)
   {
	   return FALSE;
   }
//   AccNode = inPacket.SearchElement("/所有摄像头/Domains/Domain/Cameras/Camera");
	AccNode = inPacket.SearchElement("/所有摄像头");
	AccNode = inPacket.SearchElement("Domains");
	AccNode = inPacket.SearchElement("Domain");
	AccNode = inPacket.SearchElement("Cameras");
	AccNode = inPacket.SearchElement("Camera");
   if(!AccNode)
   {
	   return FALSE;
   }
   inPacket.SetCurrentElement(AccNode);
   while(AccNode)
   {
		if((AccNode1 = inPacket.SearchElement("strCameraID")) == NULL)
			return FALSE;
		ATL::CString str;
		if(strcmp(channelid,AccNode1->getTextContent()) != 0)
		{
			AccNode = AccNode->GetNextElement();
			inPacket.SetCurrentElement(AccNode);
			continue;
		}


		if((AccNode2 = inPacket.SearchElement("strStoragePoint")) == NULL)
			return FALSE;

		str.Format("%s",AccNode2->getTextContent());
		if(str == "(null)")
			return FALSE;
		sprintf(strpath,"%s\\%s",AccNode2->getTextContent(),channelid);
		break;

		AccNode = AccNode->GetNextElement();
		inPacket.SetCurrentElement(AccNode);
   }  
   if(strcmp(strpath,"") == 0)
   {
	   return FALSE;
   }   

	CTime starttime, endtime;
	//Ticks2Time(starttimepos, starttime);
	//Ticks2Time(endtimepos, endtime);
	starttime=endtime=0;
	char indexfilename[MAX_PATH];
//	sprintf(indexfilename, "%s\\%s.idx", strpath,starttime.Format("%Y-%m-%d") );

	HANDLE hindexfile = INVALID_HANDLE_VALUE;

	CTime curpostime = starttime;

	//IDXFILEHEAD_INFO fileheadinfo;

	BOOL bstart = FALSE;
		
	FILE_INFO fileinfo;
	while(1)
	{
		ATL::CString str = curpostime.Format("%Y%m%d");
		sprintf(indexfilename, "%s\\%s.idx", strpath ,str.GetBuffer(str.GetLength()) );
		str.ReleaseBuffer();

		SECURITY_ATTRIBUTES sa;
		sa.nLength              = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle       = 1;

		ATL::CString stridxfile;
		stridxfile.Format("%s",indexfilename );

		hindexfile = ::CreateFile(stridxfile , 
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(hindexfile == INVALID_HANDLE_VALUE)
		{
			CTimeSpan spantime = CTimeSpan::CTimeSpan(1,0,0,0);

			curpostime = curpostime + spantime;
			if(curpostime > endtime)
				return TRUE;
			continue;
		}

		while(1)
		{
			IDXSTAMP_INFO stampinfo;

			if(!ReadNextTimeStampInfo(hindexfile, stampinfo))
				break;
			if(starttimepos > stampinfo.time)
			{
				continue;

			}
			if(endtimepos < stampinfo.time)
			{
				if(bstart == FALSE)
				{
					return FALSE;
				}

				char filename[MAX_PATH];
				sprintf(filename, "%s\\%d.mp4", strpath,stampinfo.blockindex);
				if(strcmp(filename, fileinfo.FileName) == 0)
				{
					int size = m_FileArray.GetSize();
					m_FileArray[size-1].endpos = stampinfo.pos;
				}
				
				if(hindexfile != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(hindexfile);
					hindexfile = INVALID_HANDLE_VALUE;
				}

				return TRUE;
			}
/////////////////插入文件列表
			FILESTAMP_INFO filestampinfo;
			sprintf(filestampinfo.filename, "%s\\%d.mp4", strpath,stampinfo.blockindex);
			filestampinfo.pos = stampinfo.pos;
			filestampinfo.time = stampinfo.time;
			m_StampArray.Add(filestampinfo);

			if(bstart == FALSE)
			{

				fileinfo.startpos = stampinfo.pos;
				fileinfo.endpos = -1;
				sprintf(fileinfo.FileName, "%s\\%d.mp4", strpath,stampinfo.blockindex);

				bstart = TRUE;
				m_FileArray.Add(fileinfo);


			}else{
				char filename[MAX_PATH];
				sprintf(filename, "%s\\%d.mp4", strpath,stampinfo.blockindex);
				if(strcmp(filename, fileinfo.FileName) == 0)
				{

				}else{
					fileinfo.startpos = -1;
					fileinfo.endpos = -1;
					sprintf(fileinfo.FileName, "%s\\%d.mp4", strpath,stampinfo.blockindex);
					m_FileArray.Add(fileinfo);
				}
			}
		}

		if(hindexfile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hindexfile);
			hindexfile = INVALID_HANDLE_VALUE;
		}

		CTimeSpan spantime = CTimeSpan::CTimeSpan(1,0,0,0);

		curpostime = curpostime + spantime;
		if(curpostime > endtime)
			return TRUE;

		return TRUE;

	}
	return TRUE;
}

BOOL CFileIndex::ReadIdxFileInfo(HANDLE hfile,IDXFILEHEAD_INFO &info)
{
	if(hfile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	SetFilePointer(hfile, 0,NULL, FILE_BEGIN);
	int filelength = GetFileSize(hfile, NULL);
	if(filelength <= sizeof(IDXFILEHEAD_INFO))
	{
		return FALSE;
	}

	DWORD readlength;
	if(!ReadFile(hfile,&info.startblock,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.endblock,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.BlockFileNum,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CFileIndex::ReadNextBlockInfo(HANDLE hfile,IDXFILEBLOCK_INFO &info)
{
	if(hfile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	DWORD readlength;
	if(!ReadFile(hfile,&info.starttime,8,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 8)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.endtime,8,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 8)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.startblock,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.endblock,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.stampnum,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CFileIndex::ReadNextTimeStampInfo(HANDLE hfile,IDXSTAMP_INFO &info)
{
	if(hfile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	DWORD readlength;
	if(!ReadFile(hfile,&info.blockindex,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.pos,4,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 4)
	{
		return FALSE;
	}

	if(!ReadFile(hfile,&info.time,8,&readlength,NULL))
	{
		return FALSE;
	}
	if(readlength < 8)
	{
		return FALSE;
	}

	return TRUE;

}
