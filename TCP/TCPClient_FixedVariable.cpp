#pragma comment(lib,"ws2_32") // ���� �Լ���
#pragma comment(lib,"user32") // �޽��� �ڽ� �Լ�

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 50

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
	// initialize Winsock
	WSADATA wsa;
	if (0!=WSAStartup(MAKEWORD(2,2), &wsa)) return 1;
	
	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET==sock) err_quit("main().socket()");
	
	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	int retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (SOCKET_ERROR == retval) err_quit("main().connect()");
	
	char buf[BUFSIZE];
	char *testdata[] = {
		"�ȳ��ϼ���.",
		"�ݰ�����.",
		"���� ���� �� �̾߱Ⱑ ���� �� ���׿�.",
		"���� �׷��׿�.",
	};
	
	// data communication with server
	int len;
	for (int i=0;4>i;i++)
	{
		len=strlen(testdata[i]);
		strncpy(buf, testdata[i], len);
		
		// send data (fixed)
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("main.send()");
			break;
		}
		
		// send data (variable)
		retval = send(sock, buf, len, 0);
		if (SOCKET_ERROR == retval)
		{
			err_display("main.send()");
			break;
		}

		fprintf(stdout, "[TCP Ŭ���̾�Ʈ] %d+%d ����Ʈ�� ���½��ϴ�.\n", sizeof(int), retval);
	}
	
	closesocket(sock);

	WSACleanup();
	return 0;
}
