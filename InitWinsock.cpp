#pragma comment(lib,"ws2_32") // 윈속 함수들
#pragma comment(lib,"user32") // 메시지 박스 함수

#include <winsock2.h>

int main()
{
  WSADATA wsa;
  if (0!=WSAStartup(MAKEWORD(2,2),&wsa)) return 1;
  MessageBox(NULL,"윈속 초기화 성공","알림",MB_OK);
  WSACleanup();

  return 0;
}

