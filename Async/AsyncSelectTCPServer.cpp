#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")
#pragma comment(lib,"gdi32")

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

void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,errcode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
	fprintf(stderr, "[Error] %s\n",(char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

const int BUFSIZE=512;

struct SOCKETINFO
{
  SOCKET sock;
  char buf[BUFSIZE+1];
  int recvbytes;
  int sendbytes;
  BOOL recvdelayed;
  SOCKETINFO *next;
};

SOCKETINFO *SocketInfoList;

BOOL AddSocketInfo(SOCKET sock)
{
  SOCKETINFO *ptr = new SOCKETINFO;
  if (NULL==ptr)
  {
    err_display("Out of memory for SOCKETINFO.");
    return FALSE;
  }
  ptr->sock=sock;
  ptr->recvbytes=0;
  ptr->sendbytes=0;
  ptr->recvdelayed=FALSE;
  ptr->next=SocketInfoList;
  SocketInfoList=ptr;
  return TRUE;
}

SOCKETINFO *GetSocketInfo(SOCKET sock)
{
  SOCKETINFO *ptr=SocketInfoList;
  while (ptr)
  {
    if (ptr->sock==sock) return ptr;
    ptr=ptr->next;
  }
  return NULL;
}

void RemoveSocketInfo(SOCKET sock)
{
  SOCKADDR_IN clientaddr;
  int addrlen = sizeof(clientaddr);
  getpeername(sock,(SOCKADDR *)&clientaddr,&addrlen);
  fprintf(stdout, "[TCP Server] Client termination: IP address=%s, Port no.=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
  SOCKETINFO *curr = SocketInfoList;
  SOCKETINFO *prev = NULL;
  while (curr)
  {
    if (curr->sock==sock)
    {
      if (prev) prev->next=curr->next; else SocketInfoList=curr->next;
      closesocket(curr->sock);
      delete curr;
      return;
    }
    prev=curr;
    curr=curr->next;
  }
}

const int WM_SOCKET=WM_USER+1;

void ProcessSocketMessage(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  if (WSAGETSELECTERROR(lParam))
  {
    err_display(WSAGETSELECTERROR(lParam));
    RemoveSocketInfo(wParam);
    return;
  }
  int addrlen;
  SOCKADDR_IN clientaddr;
  SOCKET client_sock;
	SOCKETINFO *ptr;
	int retval;
  switch (WSAGETSELECTEVENT(lParam))
  {
    case FD_ACCEPT:
      addrlen=sizeof(clientaddr);
      client_sock=accept(wParam,(SOCKADDR*)&clientaddr,&addrlen);
      if (INVALID_SOCKET==client_sock)
			{
				err_display("accept()");
				return;
			}
			fprintf(stdout, "\n[TCP Server] Client connected : IP Address=%s, Port no=%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
			AddSocketInfo(client_sock);
			if (SOCKET_ERROR==WSAAsyncSelect(client_sock,hWnd,WM_SOCKET,FD_READ|FD_WRITE|FD_CLOSE))
			{
				err_display("WSAAsyncSelect()");
				RemoveSocketInfo(client_sock);
			}
      break;
    case FD_READ:
			ptr=GetSocketInfo(wParam);
			if (0<ptr->recvbytes)
			{
				ptr->recvdelayed=TRUE;
				return;
			}
			retval=recv(ptr->sock,ptr->buf,BUFSIZE,0);
			if (SOCKET_ERROR==retval)
			{
				err_display("recv()");
				RemoveSocketInfo(wParam);
				return;
			}
			ptr->recvbytes=retval;
			ptr->buf[retval]='\0';
			addrlen=sizeof(clientaddr);
			getpeername(wParam,(SOCKADDR*)&clientaddr,&addrlen);
			fprintf(stdout, "[TCP/%s:%d]%s\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port),ptr->buf);
      break;
    case FD_WRITE:
			ptr=GetSocketInfo(wParam);
			if (ptr->recvbytes<=ptr->sendbytes) return;
			retval=send(ptr->sock, ptr->buf + ptr->sendbytes,ptr->recvbytes - ptr->sendbytes,0);
			if (SOCKET_ERROR==retval)
			{
				err_display("send()");
				RemoveSocketInfo(wParam);
				return;
			}
			ptr->sendbytes+=retval;
			if (ptr->recvbytes==ptr->sendbytes)
			{
				ptr->recvbytes=ptr->sendbytes=0;
				if (ptr->recvbytes)
				{
					ptr->recvdelayed=FALSE;
					PostMessage(hWnd,WM_SOCKET,wParam,FD_READ);
				}
			}
      break;
    case FD_CLOSE:
      RemoveSocketInfo(wParam);
      break;
  }
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_SOCKET:
      ProcessSocketMessage(hWnd,uMsg,wParam,lParam);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

const int SERVERPORT=9000;

int main(int argc, char const *argv[])
{
	WNDCLASS wndlass;
	wndlass.style=CS_HREDRAW|CS_VREDRAW;
	wndlass.lpfnWndProc=WndProc;
	wndlass.cbClsExtra=0;
	wndlass.cbWndExtra=0;
	wndlass.hInstance=NULL;
	wndlass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wndlass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndlass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndlass.lpszMenuName=NULL;
	wndlass.lpszClassName="MyWndClass";
	if (!RegisterClass(&wndlass)) return 1;
	HWND hWnd 
  return 0;
}
