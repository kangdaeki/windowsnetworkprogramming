#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,WSAGetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,WSAGetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
	fprintf(stderr, "[%s] %s\n",msg,(char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

const int BUFSIZE=512;

struct SOCKETINFO
{
  SOCKET sock;
  char buf[BUFSIZE+1];
  int recvbytes;
  int sendbytes;
};

SOCKETINFO *SocketInfoArray[FD_SETSIZE];

int nTotalSockets=0;

BOOL AddSocketInfo(SOCKET sock)
{
  if (FD_SETSIZE<=nTotalSockets)
  {
//    fprintf(stdout, "[Error] Cannot add socket info, SOCKETINFO.\n");
    err_display("Cannot add socket info, SOCKETINFO.");
    return FALSE;
  }
  SOCKETINFO *ptr = new SOCKETINFO;
  if (NULL==ptr)
  {
//    fprintf(stdout, "[Error] Out of memory for SOCKETINFO.\n");
    err_display("Out of memory for SOCKETINFO.");
    return FALSE;
  }
  ptr->sock=sock;
  ptr->recvbytes=0;
  ptr->sendbytes=0;
  SocketInfoArray[nTotalSockets]=ptr;
  nTotalSockets++;
  return TRUE;
}

void RemoveSocketInfo(int nIndex)
{
  SOCKETINFO *ptr = SocketInfoArray[nIndex];
  SOCKADDR_IN clientaddr;
  int addrlen = sizeof(clientaddr);
  getpeername(ptr->sock,(SOCKADDR *)&clientaddr,&addrlen);
  fprintf(stdout, "[TCP Server] Client termination: IP address=%s, Port no.=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
  if (nIndex!=(nTotalSockets-1)) SocketInfoArray[nIndex]=SocketInfoArray[nTotalSockets-1];
  --nTotalSockets;
}

const int SERVERPORT=9000;

int main(int argc, char *argv[])
{
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;
  SOCKET listen_sock = socket(AF_INET,SOCK_STREAM,0);
  if (INVALID_SOCKET==listen_sock) err_quit("socket()");
  SOCKADDR_IN serveraddr;
  ZeroMemory(&serveraddr,sizeof(serveraddr));
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
  serveraddr.sin_port=htons(SERVERPORT);
  if (SOCKET_ERROR==bind(listen_sock,(SOCKADDR *)&serveraddr,sizeof(serveraddr))) err_quit("bind()");
  if (SOCKET_ERROR==listen(listen_sock,SOMAXCONN)) err_quit("listen()");
  u_long on=1;
  if (SOCKET_ERROR==ioctlsocket(listen_sock,FIONBIO,&on)) err_display("ioctlsocket()");
  while (1)
  {
    FD_SET rset, wset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(listen_sock,&rset);
    for (int i=0;i<nTotalSockets;i++)
      if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes) FD_SET(SocketInfoArray[i]->sock,&wset);
      else FD_SET(SocketInfoArray[i]->sock,&rset);
    if (SOCKET_ERROR==select(0,&rset,&wset,NULL,NULL)) err_quit("select()");
    // (1): accepting client connection.
    if (FD_ISSET(listen_sock,&rset))
    {
      SOCKADDR_IN clientaddr;
      int addrlen=sizeof(clientaddr);
      SOCKET client_sock=accept(listen_sock,(SOCKADDR *)&clientaddr,&addrlen);
      if (INVALID_SOCKET==client_sock) err_display("accept()");
      else
      {
        fprintf(stdout, "\n[TCP Server] Client connected: IP address=%s, Port no=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        AddSocketInfo(client_sock);
      }
    }
    // (2): data communication
    for (int i=0;i<nTotalSockets;i++)
    {
      SOCKETINFO *ptr=SocketInfoArray[i];
      if (FD_ISSET(ptr->sock,&rset))
      {
        int retval=recv(ptr->sock,ptr->buf,BUFSIZE,0);
        if (SOCKET_ERROR==retval)
        {
          err_display("recv()");
          RemoveSocketInfo(i);
          continue;
        }
        else if (0==retval)
        {
          RemoveSocketInfo(i);
          continue;
        }
        ptr->recvbytes=retval;

        SOCKADDR_IN clientaddr;
        int addrlen=sizeof(clientaddr);
        getpeername(ptr->sock,(SOCKADDR *)&clientaddr,&addrlen);
        ptr->buf[retval]='\0';
        fprintf(stdout, "\n[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ptr->buf);
      }
      if (FD_ISSET(ptr->sock,&wset))
      {
        int retval=send(ptr->sock,ptr->buf+ptr->sendbytes,ptr->recvbytes-ptr->sendbytes,0);
        if (SOCKET_ERROR==retval)
        {
          err_display("send()");
          RemoveSocketInfo(i);
          continue;
        }
        ptr->sendbytes+=retval;
        if (ptr->recvbytes==ptr->sendbytes) ptr->recvbytes=ptr->sendbytes=0;
      }
    }
  }
  WSACleanup();
  return 0;
}
