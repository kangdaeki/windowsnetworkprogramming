#pragma comment(lib,"ws2_32") // ���� �Լ���
//#pragma comment(lib,"user32") // �޽��� �ڽ� �Լ�

#include <winsock2.h>
#include <stdio.h>

int main()
{
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;

  u_short x1=0x1234; // ȣ��Ʈ
  u_long y1=0x12345678; 

  u_short x2=htons(x1); // ��Ʈ��ũ
  u_long y2=htonl(y1); 

  fprintf(stdout, "[ȣ��Ʈ ����Ʈ --> ��Ʈ��ũ ����Ʈ]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x1,x2);
  fprintf(stdout, "0x%x --> 0x%x\n",y1,y2);

  x1=ntohs(x2);
  y1=ntohl(y2);
  fprintf(stdout, "[��Ʈ��ũ ����Ʈ --> ȣ��Ʈ ����Ʈ]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x2,x1);
  fprintf(stdout, "0x%x --> 0x%x\n",y2,y1);

  fprintf(stdout, "[short �� long ���� ������ ���]\n");
  fprintf(stdout, "0x%x --> 0x%x\n",x1,htonl(x1));
  WSACleanup();

  return 0;
}

