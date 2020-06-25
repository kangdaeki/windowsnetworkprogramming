#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

const int BUF_SIZE=100;

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	for (int i=idx;i<total;i++) hSockArr[i]=hSockArr[i+1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	for (int i=idx;i<total;i++) hEventArr[i]=hEventArr[i+1];
}

void ErrorHandling(char *msg)
{
	fprintf(stderr, "%s\n",msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if (2!=argc)
	{
		fprintf(stderr, "Usage: %s <port>\n",argv[0]);
		exit(1);
	}
	WSADATA wsaData;
	if (0!=WSAStartup(MAKEWORD(2,2), &wsaData)) ErrorHandling("WSAStartup() error!");

	SOCKET hServSock= socket(PF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (SOCKET_ERROR == bind(hServSock, (SOCKADDR *)&servAdr, sizeof(servAdr))) ErrorHandling("bind() error!");

	if (SOCKET_ERROR == listen(hServSock, 5)) ErrorHandling("listen() error!");

	WSAEVENT newEvent = WSACreateEvent();
	if (SOCKET_ERROR == WSAEventSelect(hServSock, newEvent, FD_ACCEPT)) ErrorHandling("listen() error!");

	int numOfClntSock=0;
	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	hSockArr[numOfClntSock]=hServSock;
	hEventArr[numOfClntSock]=newEvent;
	numOfClntSock++;

	while (1)
	{
		int posInfo=WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
		int startIdx = posInfo-WSA_WAIT_EVENT_0;
		for (int i=startIdx; i<numOfClntSock;i++)
		{
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
			if ((WSA_WAIT_FAILED==sigEventIdx) ||(WSA_WAIT_TIMEOUT == sigEventIdx)) continue;
			else
			{
				sigEventIdx = i;
				WSANETWORKEVENTS netEvents;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
				if (netEvents.lNetworkEvents & FD_ACCEPT)
				{
					if (0!=netEvents.iErrorCode[FD_ACCEPT_BIT])
					{
						fprintf(stderr, "Accept Error\n");
						break;
					}
					SOCKADDR_IN clntAdr;
					int clntAdrLen=sizeof(clntAdr);
					SOCKET hClntSock=accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
					newEvent=WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ|FD_CLOSE);
					hEventArr[numOfClntSock]=newEvent;
					hSockArr[numOfClntSock]=hClntSock;
					numOfClntSock++;
					fprintf(stdout, "New client connected. %d\n",hClntSock);
				}
				if (netEvents.lNetworkEvents & FD_READ)
				{
					if (0!=netEvents.iErrorCode[FD_READ_BIT])
					{
						fprintf(stderr, "Read Error\n");
						break;
					}
					char msg[BUF_SIZE];
					int strLen=recv(hSockArr[sigEventIdx],msg,sizeof(msg),0);
					send(hSockArr[sigEventIdx], msg, strLen, 0);
				}
				if (netEvents.lNetworkEvents & FD_CLOSE)
				{
					if (0!=netEvents.iErrorCode[FD_CLOSE_BIT])
					{
						fprintf(stderr, "Close Error\n");
						break;
					}
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					numOfClntSock--;
					CompressSockets(hSockArr,sigEventIdx,numOfClntSock);
					CompressEvents(hEventArr,sigEventIdx,numOfClntSock);
				}
			}
		}
	}

	WSACleanup();
	return 0;
}
