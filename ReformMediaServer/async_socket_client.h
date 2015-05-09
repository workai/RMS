#ifndef _ASYNC_SOCKET_CLIENT_H_
#define _ASYNC_SOCKET_CLIENT_H_
#include <WinSock2.h>
#include "media_protocal.h"
#include <vector>

using namespace std;

struct DataBuffer
{
	int len;//
	char* data;//
};

class AsyncSocketClient
{
public:
	AsyncSocketClient(){}
	AsyncSocketClient(SOCKET s);
	void release();
	
	SOCKET getSocket() {return _socket;}
	//读取消息后返回TRUE，否则返回FALSE ,内存需要手动释放(delete [] *buf)
	int recvMSG(char** buf);

	//发送消息，成功返回TRUE，未完全发送返回FALSE，出错返回-1
	int sendMSG(char* const buf, const int len);

	//发送未发送完成的数据，不确保发送完成
	int flushSendData() {return sendData();}

private:
	bool _finish_head;
	bool _finish_body;
	int _num_readed;
	int _num_message;
	PacketHeader* _header;
	char* _recvbuf;//接收缓存
	char* _testbuf;
	
	bool _finish_send;//当前包的发送状态
	int _num_send;//当前包已发送的大小
	vector<DataBuffer> _sendbuf_vector;//发送缓存

	SOCKET _socket;

private:
	int readData(char* const buf, const int len);
	bool finishHeader();
	void beginHeader();

	int sendData();
};




#endif