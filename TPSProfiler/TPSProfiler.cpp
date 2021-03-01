#include "stdafx.h"
#include <process.h>
#include "CTPSProfiler.h"

HANDLE gEvent;

DWORD WINAPI WorkerThread(void* pParam)
{
	WaitForSingleObject(gEvent, INFINITE);

	CTPSProfiler::PrintTPSProfile();

	return 1;
}


int wmain()
{
	srand((unsigned)time(NULL));

	CreateEvent(NULL, false, false, NULL);

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)WorkerThread, NULL, NULL, NULL);

	CTPSProfiler::SetTPSProfiler(L"TEST");

	CTPSProfiler profiler;

	DWORD time = timeGetTime();

	while (timeGetTime() - time < 4000)
	{
		profiler.SaveTPSInfo(L"TEST_TPS", rand() % 10000);

		if (timeGetTime() - time > 1000)
		{
			SetEvent(gEvent);
		}
	}

	CTPSProfiler::PrintTPSProfile();

	CTPSProfiler::FreeTPSProfiler();

	return 1;
}
