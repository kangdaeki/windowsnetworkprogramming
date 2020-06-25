#include <windows.h>
#include <stdio.h>

#define BUFSIZE 10

HANDLE hReadEvent;
HANDLE hWriteEvent;
int buf[BUFSIZE];

DWORD WINAPI WriteThread(LPVOID arg)
{
	DWORD retval;
	for (int k=1; k<=500; k++)
	{
		retval = WaitForSingleObject(hReadEvent, INFINITE);
		if (WAIT_OBJECT_0 != retval) break;
		for (int i=0; i<BUFSIZE; i++) buf[i]=k;
		SetEvent(hWriteEvent);
	}
	return 0;
}

DWORD WINAPI ReadThread(LPVOID arg)
{
	DWORD retval;
	while (1)
	{
		retval = WaitForSingleObject(hWriteEvent, INFINITE);
		if (WAIT_OBJECT_0 != retval) break;
		fprintf(stdout, "Thread %4d: ", GetCurrentThreadId());
		for (int i=0; i<BUFSIZE; i++) fprintf(stdout, "%3d  ", buf[i]);
		fprintf(stdout, "\n");
		ZeroMemory(buf, sizeof(buf));
		SetEvent(hReadEvent);
	}
	return 0;
}

int main()
{
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == hWriteEvent) return 1;
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (NULL == hReadEvent) return 1;
	
	HANDLE hThreads[3];
	hThreads[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThreads[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThreads[2] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);

	WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);

	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);

	return 0;
}



