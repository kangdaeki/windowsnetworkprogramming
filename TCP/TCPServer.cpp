#pragma comment(lib,"ws2_32") // 윈속 함수들
#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

const int SERVERPORT=9000;
const int BUFSIZE=512;

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

int main(int argc, char *argv[])
{
	int retval;
	
	WSADATA wsa;
	if (0!=WSAStartup(MAKEWORD(2,2), &wsa)) return 1;
	
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET==listen_sock) err_quit("main().socket()");
	
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("main().bind()");
	
	retval = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == retval) err_quit("main().listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];
	
	while (1)
	{
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
			retval = recv(client_sock,buf,BUFSIZE,0);
			if (SOCKET_ERROR==retval)
			{
				err_display("main().recv()");
				break;
			}
			else if (0==retval) break;
			
			buf[retval] ='\0';
			fprintf(stdout, "[TCP %s:%d] %s\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);
			
			retval = send(client_sock,buf,retval,0);
			if (SOCKET_ERROR==retval)
			{
				err_display("main().send()");
				break;
			}
		}
		closesocket(client_sock);
		fprintf(stdout, "[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}
	
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
