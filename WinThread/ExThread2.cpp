#include <windows.h>
#include <stdio.h>

DWORD WINAPI MyThread(LPVOID arg)
{
	while (1) ;
	return 0;
}

int main (int argc, char *argv[])
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	
	fprintf(stdout, "# of CPUs = %d\n",(int)si.dwNumberOfProcessors);
	for (int i=0; i<(int)si.dwNumberOfProcessors; i++)
	{
		HANDLE hThread = CreateThread(NULL, 0, MyThread, NULL, 0, NULL);
		if (NULL == hThread) return 1;
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		CloseHandle(hThread);
	}
	
	Sleep(1000);
	
	fprintf(stdout, "Running main()\n");
	return 0;
}
