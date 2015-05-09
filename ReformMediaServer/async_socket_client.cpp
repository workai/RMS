#include "async_socket_client.h"
#include <iostream>

AsyncSocketClient::AsyncSocketClient(SOCKET s)
	:_socket(s)
{
	_recvbuf = new char[256];
	memset(_recvbuf,0,256);
	_header = (PacketHeader*)_recvbuf;
	//_header = new PacketHeader();
	//_recvbuf = NULL;
	_finish_head = true;
	_finish_body = true;
	_num_message = 0;
	_num_readed = 0;
	_num_send = 0;
	_finish_send = true;
}

int AsyncSocketClient::recvMSG(char** buf)
{
	if(_socket == INVALID_SOCKET)
		return SOCKET_ERROR;
	int ret = 0;
	//read header
	if(_finish_body && _finish_head)
	{
		beginHeader();
		ret = readData((char*)_header, _num_message);
		if(ret != TRUE)
			return ret;
		else if(!finishHeader())
		{
			_num_readed=0;
			return FALSE;
		}
	}
	else
	{
		ret = readData((char*)_header+_num_readed, _num_message-_num_readed);
		if(ret != TRUE)
			return ret;
		else if(!finishHeader())
		{
			_num_readed=0;
			return FALSE;
		}
	}

	//read body
	ret = readData(_recvbuf+_num_readed, _num_message-_num_readed);
	if(ret == TRUE)
	{
		_finish_body = true;
		*buf = _recvbuf;
	}
	return ret;
}

int AsyncSocketClient::sendMSG(char* const buf, const int len)
{
	if(_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	if(len < 1)
		return TRUE;
	DataBuffer dbf;
	dbf.len = len;
	dbf.data = new char[len];
	memcpy(dbf.data, buf, len);
	_sendbuf_vector.push_back(dbf);

	return sendData();
}

int AsyncSocketClient::sendData()
{
	if(_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	for(vector<DataBuffer>::iterator iter=_sendbuf_vector.begin();iter!=_sendbuf_vector.end();)
	{
		DataBuffer dbf = *iter;
		while(_num_send < iter->len)
		{
			int ret = send(_socket, dbf.data, dbf.len - _num_send, 0);
			if(ret > 0)
			{
				_num_send += ret;
			}
			else
			{
				DWORD err = GetLastError();
				if(err == WSAEWOULDBLOCK)
					return FALSE;
				else
				{
					closesocket(_socket);
					_socket = INVALID_SOCKET;
					//printf("send err:%d, close socket\n", err);
					return -1;
				}
			}
		}
		_num_send=0;
		delete [] dbf.data;
		iter = _sendbuf_vector.erase(iter);
	}

	return TRUE;
}

int AsyncSocketClient::readData(char* const buf, const int len)
{
	if(_socket == INVALID_SOCKET)
		return SOCKET_ERROR;

	int ret = 0;
	int get_len = len;
	while(get_len)
	{
		ret = recv(_socket, buf, get_len, 0);
		if(ret == SOCKET_ERROR)
		{
			DWORD err = GetLastError();
			switch(err)
			{
			case WSAEWOULDBLOCK:
				{
					return FALSE;
				}
				break;
			default:
				{
					closesocket(_socket);
					_socket = INVALID_SOCKET;
					//printf("read err:%d, close socket\n", err);
				}
				return -1;
			}
		}
		else if(ret == 0)
		{
			//close socket
			//printf("socket close\n");
			closesocket(_socket);
			_socket = INVALID_SOCKET;
			return -1;
		}
		else
		{
			get_len-= ret;
			_num_readed+=ret;
		}
	}
	return TRUE;
}

void AsyncSocketClient::release()
{
}

bool AsyncSocketClient::finishHeader()
{
	bool ret=false;
	if(_header->nPacketID == 0xABCD)
	{
		ret=true;
		
		_finish_body = false;
		_finish_head = true;
		_num_readed = sizeof(PacketHeader);//0
		_num_message = _header->nDataSize+_num_readed;		
	}
	else
		std::cout<<"MSG UN recognition"<<std::endl;
	return ret;
}

void AsyncSocketClient::beginHeader()
{
	_finish_head = false;
	_num_message = sizeof(PacketHeader);
	_num_readed = 0;
}

