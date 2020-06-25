#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

const int BUF_SIZE=1024;

void ErrorHandling(char *msg)
{
	fprintf(stderr, "%s\n",msg);
	exit(1);
}

int main(int argc, char const *argv[])
{
  if (2!=argc)
  {
    fprintf(stderr, "USAGE: %s <port>\n",argv[0]);
  	exit(1);
  }
  WSADATA wsaData;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsaData)) ErrorHandling("WSAStartup() error!");

  SOCKADDR_IN lisnAdr;
  memset(&lisnAdr, 0, sizeof(lisnAdr));
  lisnAdr.sin_family = AF_INET;
  lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
  lisnAdr.sin_port = htons(atoi(argv[1]));
  SOCKET hLisnSocket=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

  if (SOCKET_ERROR==bind(hLisnSocket,(SOCKADDR*)&lisnAdr,sizeof(lisnAdr))) ErrorHandling("bind() error!");
  if (SOCKET_ERROR==listen(hLisnSocket,5)) ErrorHandling("listen() error!");
  SOCKADDR_IN recvAdr;
  int recAdrSz=sizeof(recvAdr);
  SOCKET hRecvSocket=accept(hLisnSocket,(SOCKADDR*)&recvAdr,&recAdrSz);

  WSAEVENT evObj=WSACreateEvent();
  WSAOVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent=evObj;
  WSABUF dataBuf;
  dataBuf.len=BUF_SIZE;
  char buf[BUF_SIZE];
  dataBuf.buf=buf;

  int recvBytes=0;
  int flags=0;
  if (SOCKET_ERROR==WSARecv(hRecvSocket,&dataBuf,1,(LPDWORD)&recvBytes,(LPDWORD)&flags,&overlapped,NULL))
  {
    if (WSA_IO_PENDING==WSAGetLastError())
    {
      fprintf(stdout, "Background data receive!\n");
      WSAWaitForMultipleEvents(1,&evObj,TRUE,WSA_INFINITE,FALSE);
      WSAGetOverlappedResult(hRecvSocket,&overlapped,(LPDWORD)&recvBytes,FALSE,NULL);
    }
    else ErrorHandling("WSARecv() error!");
  }

  fprintf(stdout, "Received message: %s\n",buf);
  WSACloseEvent(evObj);
  closesocket(hRecvSocket);
  closesocket(hLisnSocket);
  WSACleanup();
  return 0;
}
