#pragma comment(lib,"ws2_32") // 윈속 함수들

#include <winsock2.h>
#include <stdio.h>

#define TESTNAME "www.google.com"

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

BOOL GetIPAddr(char *name, IN_ADDR *addr)
{
	HOSTENT *ptr=gethostbyname(name);
	if (NULL==ptr)
	{
		err_display("GetIPAddr().gethostbyname()");
		return FALSE;
	}
	if (AF_INET!=ptr->h_addrtype) return FALSE;
	memcpy(addr, ptr->h_addr, ptr->h_length);
	return TRUE;
}

BOOL GetDomainName(IN_ADDR addr, char *name, int namelen)
{
	HOSTENT *ptr=gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
	if (NULL==ptr)
	{
		err_display("GetDomainName().gethostbyaddr()");
		return FALSE;
	}
	if (AF_INET!=ptr->h_addrtype) return FALSE;
	strncpy(name, ptr->h_name, namelen);
	return TRUE;
}

int main(int argc, char *argv[])
{
	
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2), &wsa)) return 1;
  
  fprintf(stdout,"도메인 이름 (변환 전) = %s\n",TESTNAME);
  IN_ADDR addr;
  if (GetIPAddr(TESTNAME, &addr))
  {
	  fprintf(stdout,"IP 주소 (변환 후) = %s\n",inet_ntoa(addr));
	  char name[256];
	  if (GetDomainName(addr, name, sizeof(name))) fprintf(stdout,"도메인 이름 (다시 변환 후) = %s\n",name);
  }
  
  WSACleanup();
  return 0;
}

