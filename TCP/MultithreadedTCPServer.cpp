#pragma comment(lib,"ws2_32") // 윈속 함수들
#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 512

// Print error information of the socket function and terminate
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

// Print error information of the socket function
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

// Data communication with the client
DWORD WINAPI ProcessClient(LPVOID arg)
{
	// Get client information
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	SOCKET client_sock = (SOCKET) arg;
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
	int retval;
	char buf[BUFSIZE+1];
	while (1)
	{
		// Get data
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("recv()");
			break;
		} else if (0==retval) break;
		// Print out the data
		buf[retval]='\0';
		fprintf(stdout, "[TCP %s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);
		// Send the data
		retval = send(client_sock, buf, retval, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("send()");
			break;
		}
	}
	closesocket(client_sock);
	fprintf(stdout, "[TCP Server] Client terminated: IP address = %s, port number = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	return 0;
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
	HANDLE hThread;

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
		// Print information about the connected client
		fprintf(stdout, "\n");
		fprintf(stdout, "[TCP Server] Client connected: IP 주소=%s, 포트 번호=%d\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		// Create a thread
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (NULL==hThread) closesocket(client_sock); else CloseHandle(hThread);
	}
	
	closesocket(listen_sock);
	WSACleanup();

	return 0;
}
