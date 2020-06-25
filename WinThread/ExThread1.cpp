#include <windows.h>
#include <stdio.h>

struct Point3D
{
	int x,y,z;
};

DWORD WINAPI MyThread(LPVOID arg)
{
	Point3D *pt = (Point3D *)arg;
	while (1)
	{
		fprintf(stdout, "Running MyThread() %d : %d, %d, %d\n",GetCurrentThreadId(), pt->x, pt->y, pt->z);
		Sleep(1000);
	}
	return 0;
}

int main (int argc, char *argv[])
{
	Point3D pt1 = {10,20,30};
	HANDLE hThread1 = CreateThread(NULL, 0, MyThread, &pt1, 0, NULL);
	if (NULL == hThread1) return 1;
	CloseHandle(hThread1);
	
	Point3D pt2 = {40,50,60};
	HANDLE hThread2 = CreateThread(NULL, 0, MyThread, &pt2, 0, NULL);
	if (NULL == hThread2) return 1;
	CloseHandle(hThread2);

	while (1)
	{
		fprintf(stdout, "Running main() %d\n",GetCurrentThreadId());
		Sleep(1000);
	}
	return 0;
}
