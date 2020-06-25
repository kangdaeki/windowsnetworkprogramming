#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

const int BUF_SIZE=1024;

typedef struct
{
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags);

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClntSock;
	LPWSABUF bufInfo=&(hbInfo->wsaBuf);
	DWORD sentBytes;
	if (0==szRecvBytes)
	{
		closesocket(hSock);
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		fprintf(stdout,"Client disconnected...");
	}
	else
	{
		bufInfo->len=szRecvBytes;
		WSASend(hSock,bufInfo,1,&sentBytes,0,lpOverlapped,WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClntSock;
	LPWSABUF bufInfo=&(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo=0;
	WSARecv(hSock,bufInfo,1,&recvBytes,(LPDWORD)&flagInfo,lpOverlapped,ReadCompRoutine);
}

void ErrorHandling(char *message)
{
	fprintf(stderr,"%s\n",message);
	exit(1);
}

int main(int argc, char *argv[])
{
	if (2!=argc)
	{
		fprintf(stderr,"Usage: %s <port>",argv[0]);
		exit(1);
	}
	WSADATA wsaData;
	if (0!=WSAStartup(MAKEWORD(2,2),&wsaData)) ErrorHandling("WSAStartup() error!");
	
	return 0;
}
