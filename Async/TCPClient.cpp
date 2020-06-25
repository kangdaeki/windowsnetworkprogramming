#pragma comment(lib,"ws2_32") // 윈속 함수들
#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVERIP "127.0.0.1"
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

int recvn(SOCKET s, char *buf, int len, int flags)
{
	char *ptr=buf;
	int left=len;
	while (0<left)
	{
		int received = recv(s, ptr, left, flags);
		if (SOCKET_ERROR == received) return SOCKET_ERROR;
		else if (0 == received) break;
		left-=received;
		ptr+=received;
	}
	return len-left;
}

int main(int argc, char *argv[])
{
	int retval;
	
	if (argc!=3)
	{
		fprintf(stderr,"Usage: %s <IP address> <port>",argv[0]);
		exit(1);
	}
	
	WSADATA wsa;
	if (0!=WSAStartup(MAKEWORD(2,2), &wsa)) return 1;
	
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET==sock) err_quit("main().socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) return SOCKET_ERROR;
	
	char buf[BUFSIZE+1];
	int len;
	
	while (1)
	{
		fprintf(stdout, "\n");
		fprintf(stdout, "[보낼 데이터] ");
		if (NULL == fgets(buf, BUFSIZE+1, stdin)) break;
		
		len = strlen(buf);
		if ('\n' == buf[len-1]) buf[len-1]='\0';
		if (0 == strlen(buf)) break;
		
		retval = send(sock, buf, strlen(buf),0);
		if (SOCKET_ERROR == retval) 
		{
			err_display("main().send()");
			break;
		}
		fprintf(stdout, "[TCP 클라이언트] %d 바이트를 보냈습니다.\n", retval);
		
		retval = recvn(sock, buf, retval,0);
		if (SOCKET_ERROR == retval)
		{
			err_display("main().recvn()");
			break;
		}
		else if (0 == retval) break;
		
		buf[retval] = '\0';
		fprintf(stdout, "[TCP 클라이언트] %d 바이트를 받았습니다.\n", retval);
		fprintf(stdout, "[받은 데이터] %s\n", buf);
	}
	
	closesocket(sock);
	WSACleanup();
	return 0;
}
