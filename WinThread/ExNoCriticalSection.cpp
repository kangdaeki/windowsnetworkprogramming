#include <windows.h>
#include <stdio.h>

#define MAXCNT 100000000

int g_count = 0;

DWORD WINAPI MyThread1(LPVOID arg)
{
	for (int i=0;i<MAXCNT;i++) g_count+=2;
	return 0;
}

DWORD WINAPI MyThread2(LPVOID arg)
{
	for (int i=0;i<MAXCNT;i++) g_count-=2;
	return 0;
}

int main()
{
	HANDLE hThreads[2];
	hThreads[0]=CreateThread(NULL, 0, MyThread1, NULL, 0, NULL);
	hThreads[1]=CreateThread(NULL, 0, MyThread2, NULL, 0, NULL);
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
	fprintf(stdout, "g_count=%d\n", g_count);
	return 0;
}

