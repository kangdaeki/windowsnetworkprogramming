#pragma comment(lib,"ws2_32") // 윈속 함수들
//#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>
#include <stdio.h>

int main()
{
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;

  u_short x1=0x1234; // 호스트
  u_long y1=0x12345678; 

  u_short x2=htons(x1); // 네트워크
  u_long y2=htonl(y1); 

  fprintf(stdout, "[호스트 바이트 --> 네트워크 바이트]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x1,x2);
  fprintf(stdout, "0x%x --> 0x%x\n",y1,y2);

  x1=ntohs(x2);
  y1=ntohl(y2);
  fprintf(stdout, "[네트워크 바이트 --> 호스트 바이트]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x2,x1);
  fprintf(stdout, "0x%x --> 0x%x\n",y2,y1);

  fprintf(stdout, "[short 를 long 으로 오인한 경우]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x1,htonl(x1));
  WSACleanup();

  return 0;
}

