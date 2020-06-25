#pragma comment(lib,"ws2_32") // 윈속 함수들
#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 1024

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	fprintf(stderr, "[%s] %s\n",msg,(char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;
	
	while (0<left)
	{
		received = recv(s, ptr, left, flags);
		if (SOCKET_ERROR == received) return SOCKET_ERROR; else if (0==received) break;
		left -= received;
		ptr += received;
	}
	return (len-left);
}

int main(int argc, char *argv[])
{
	// initialize
	WSADATA wsa;
	if (0!=WSAStartup(MAKEWORD(2,2), &wsa)) return 1;
	
	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET==listen_sock) err_quit("main().socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("main().bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("main().listen()");

	// variables for data communication
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (INVALID_SOCKET == client_sock) 
		{
			err_display("main().accept()");
			break;
		}
		// 접속한 클라이언트 정보 출력
		fprintf(stdout, "\n");
		fprintf(stdout, "[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		while (1)
		{
			// get the data
			retval = recvn(client_sock,buf,BUFSIZE,0);
			if (SOCKET_ERROR==retval)
			{
				err_display("main().recv()");
				break;
			}
			else if (0==retval) break;
			buf[retval]='\0';
			fprintf(stdout, "[TCP %s:%d] %s\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);
		}
		closesocket(client_sock);
		fprintf(stdout, "[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}
	
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
