#pragma comment(lib,"ws2_32") // ���� �Լ���
#pragma comment(lib,"user32") // �޽��� �ڽ� �Լ�

#include <winsock2.h>

int main()
{
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;
  MessageBox(NULL,"���� �ʱ�ȭ ����","�˸�",MB_OK);
  WSACleanup();

  return 0;
}

