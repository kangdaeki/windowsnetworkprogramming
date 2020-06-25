#pragma comment(lib,"ws2_32") // 윈속 함수들
//#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;
  
  // IPv4 반환 연습
  char *ipv4test = "147.46.114.70";
  fprintf(stdout,"IPv4 주소(변환 전) = %s\n",ipv4test);
  IN_ADDR ipv4num;
  ipv4num.s_addr = inet_addr(ipv4test);
  fprintf(stdout,"IPv4 주소(변환 후) = 0x%x\n",ipv4num.s_addr);
  fprintf(stdout,"IPv4 주소(다시 변환 후) = %s\n",inet_ntoa(ipv4num));
  fprintf(stdout,"\n");
  
  // IPv6 반환 연습
  char *ipv6test = "2001:0230:abcd:ffab:0023:eb00:ffff:1111";
  fprintf(stdout,"IPv6 주소(변환 전) = %s\n",ipv6test);
  // WSAStringToAddress() 함수 연습
  SOCKADDR_IN6 ipv6num;
  int addrlen = sizeof(ipv6num);
  WSAStringToAddress(ipv6test, AF_INET6, NULL, (SOCKADDR *)&ipv6num, &addrlen);
  fprintf(stdout,"IPv6 주소(변환 후) = 0x");
  for (int i=0;i<16;i++) fprintf(stdout,"%02x",ipv6num.sin6_addr.u.Byte[i]);
  fprintf(stdout,"\n");
  // WSAAddressToString() 함수 연습
  char ipaddr[256];
  DWORD ipaddrlen = sizeof(ipaddr);
  WSAAddressToString((SOCKADDR *)&ipv6num, sizeof(ipv6num), NULL, ipaddr, &ipaddrlen);
  fprintf(stdout,"IPv6 주소(다시 변환 후) = %s\n",ipaddr);
  
  WSACleanup();
  return 0;
}

