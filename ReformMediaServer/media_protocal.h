#ifndef _MEDIA_PROTOCAL_H_
#define _MEDIA_PROTOCAL_H_

#pragma pack(1)
struct PacketHeader
{
	unsigned short  nPacketID;		// (0xAB,0xCD)
	unsigned short  nPacketNo;		// 包序号    
	unsigned short  nCommand;		// 指令
	unsigned int  nDataSize;		// 数据长度
};
#pragma pack()


#endif _MEDIA_PROTOCAL_H_