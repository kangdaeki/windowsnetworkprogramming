#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char *msg)
{
	fprintf(stderr, "%s\n",msg);
	exit(1);
}

int main(int argc, char const *argv[])
{
  if (3!=argc)
  {
    fprintf(stderr, "USAGE: %s <IP> <port>\n",argv[0]);
  	exit(1);
  }
  WSADATA wsaData;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsaData)) ErrorHandling("WSAStartup() error!");

  SOCKADDR_IN sendAdr;
  memset(&sendAdr, 0, sizeof(sendAdr));
  sendAdr.sin_family = AF_INET;
  sendAdr.sin_addr.s_addr = inet_addr(argv[1]);
  sendAdr.sin_port = htons(atoi(argv[2]));
  SOCKET hSocket=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

  if (SOCKET_ERROR==connect(hSocket,(SOCKADDR*)&sendAdr,sizeof(sendAdr))) ErrorHandling("connect() error!");

  WSAEVENT evObj=WSACreateEvent();
  WSAOVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent=evObj;
  WSABUF dataBuf;
  char msg[]="Network is the computer!";
  dataBuf.len=strlen(msg)+1;
  dataBuf.buf=msg;

  int sendBytes=0;
  if (SOCKET_ERROR==WSASend(hSocket,&dataBuf,1,(LPDWORD)&sendBytes,0,&overlapped,NULL))
  {
    if (WSA_IO_PENDING==WSAGetLastError())
    {
      fprintf(stdout, "Background data send!\n");
      WSAWaitForMultipleEvents(1,&evObj,TRUE,WSA_INFINITE,FALSE);
      WSAGetOverlappedResult(hSocket,&overlapped,(LPDWORD)&sendBytes,FALSE,NULL);
    }
    else ErrorHandling("WSASend() error!");
  }

  fprintf(stdout, "Send data size: %d\n",sendBytes);
  WSACloseEvent(evObj);
  closesocket(hSocket);
  WSACleanup();
  return 0;
}
