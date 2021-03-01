#pragma once

#pragma comment(lib,"Winmm.lib")

#include <strsafe.h>
#include <iostream>
#include <Windows.h>
#include <locale.h>
#include <unordered_map>
#include <vector>

class CTPSProfiler
{
public:

	class CSRWLock;

	CTPSProfiler(void)
	{
	}

	~CTPSProfiler(void)
	{
	}


	static BOOL SetTPSProfiler(const WCHAR* pTitle)
	{
		InitializeSRWLock(&mSRWLock);

		if (pTitle == nullptr)
		{
			return FALSE;
		}

		// 한글 유니코드 셋팅
		_wsetlocale(LC_ALL, L"");

		CSRWLock srwLock(&mSRWLock);

		// 저장할 파일 이름 셋팅
		mTitle = (WCHAR*)pTitle;

		return TRUE;
	}

	BOOL SaveTPSInfo(const WCHAR* pTPSName, DWORD TPS)
	{
		CSRWLock srwLock(&mSRWLock);

		if (mTitle == nullptr)
		{
			return FALSE;
		}

		stTPSProfile* pTPSProfile = findTPSProfile(pTPSName);
		if (pTPSProfile == nullptr)
		{
			pTPSProfile = new stTPSProfile;

			pTPSProfile->pTPSName = pTPSName;
			pTPSProfile->totalTPS = TPS;
			pTPSProfile->saveCount = 1;

			mTPSInfoMap.insert(std::make_pair(pTPSName, pTPSProfile));

			return TRUE;
		}

		pTPSProfile->saveCount += 1;

		pTPSProfile->totalTPS += TPS;

		return TRUE;
	}

	static void FreeTPSProfiler(void)
	{
		CSRWLock srwLock(&mSRWLock);

		mTitle = nullptr;

		for (auto& iter : mTPSInfoMap)
		{
			delete iter.second;
		}

		mTPSInfoMap.clear();
	}

	static BOOL PrintTPSProfile(void)
	{
		CSRWLock srwLock(&mSRWLock);

		if (mTitle == nullptr)
		{
			return FALSE;
		}

		WCHAR pTPSTitle[MAX_PATH] = { 0, };

		setLogTitle(pTPSTitle);

		FILE* fp = nullptr;

		_wfopen_s(&fp, pTPSTitle, L"a+t");
		if (fp == nullptr)
		{
			return FALSE;
		}

		// 열 출력.
		fwprintf_s(fp, L"TPS Name,Average,Save Count\n");

		for (auto& iter : mTPSInfoMap)
		{
			stTPSProfile* pTPSProfile = iter.second;

			DOUBLE average = ((DOUBLE)pTPSProfile->totalTPS / pTPSProfile->saveCount);

			fwprintf_s(fp, L"%s, %.6lf, %lld\n", pTPSProfile->pTPSName, average, pTPSProfile->saveCount);
		}

		fclose(fp);
	}


private:

	class CSRWLock
	{
	public:

		CSRWLock(SRWLOCK* pSRWLock)
		{
			AcquireSRWLockExclusive(pSRWLock);

			mpSRWLock = pSRWLock;
		}

		~CSRWLock(void)
		{
			ReleaseSRWLockExclusive(mpSRWLock);
		}

		SRWLOCK* mpSRWLock;
	};

	struct stTPSProfile
	{
		const WCHAR* pTPSName;

		ULONGLONG totalTPS;

		// TPS를 몇번 저장했는지
		ULONGLONG saveCount;
	};

	static void setLogTitle(WCHAR* pTPSTitle)
	{
		tm nowTime = { 0, };

		__time64_t time64 = NULL;

		errno_t retval;

		// 시스템 클록에 따라 자정 (00:00:00), 1970 년 1 월 1 일 Utc (협정 세계시) 이후 경과 된 시간 (초)을 반환 합니다.
		_time64(&time64);

		// time_t 값으로 저장 된 시간을 변환 하 고 결과를 tm형식의 구조에 저장 합니다.
		retval = _localtime64_s(&nowTime, &time64);

		StringCchPrintfW(pTPSTitle, MAX_PATH, L"[%s TPSProfile]_[%d-%02d-%02d_%02d-%02d-%02d].csv", mTitle, nowTime.tm_year + 1900, nowTime.tm_mon + 1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);

		return;
	}

	stTPSProfile* findTPSProfile(const WCHAR* pTPSName)
	{
		auto iterE = mTPSInfoMap.end();

		auto iter = mTPSInfoMap.find(pTPSName);
	
		if (iter == iterE)
		{
			return nullptr;
		}

		return iter->second;
	}
	
	inline static WCHAR* mTitle;

	inline static SRWLOCK mSRWLock;

	inline static std::unordered_map<const WCHAR*, stTPSProfile*> mTPSInfoMap;
};

