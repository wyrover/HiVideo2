#include "stdafx.h"
#include "Log.h"

namespace e
{
	CLogger::CLogger(const TCHAR* pFileName)
	{
		m_hFile = NULL;
		::InitializeCriticalSection(&m_csLock);
	}

	CLogger::~CLogger(void)
	{
		::DeleteCriticalSection(&m_csLock);
	}

	bool CLogger::Open(const TCHAR* pFileName)
	{
		if (pFileName == NULL) return false;
		::EnterCriticalSection(&m_csLock);
		if (m_hFile != NULL)
		{
			::CloseHandle(m_hFile);
		}

		m_hFile = CreateFile(pFileName
			, GENERIC_WRITE
			, 0
			, NULL
			, CREATE_NEW
			, FILE_ATTRIBUTE_NORMAL
			, NULL);

		if (m_hFile != NULL)
		{
			DWORD dwFileSize = 0;
			GetFileSize(m_hFile, &dwFileSize);
			SetFilePointer(m_hFile, dwFileSize, NULL, FILE_BEGIN);
		}

		assert(m_hFile);
		::LeaveCriticalSection(&m_csLock);
		return m_hFile != NULL;
	}

	void CLogger::Write(const TCHAR* pszFormat, va_list arglist)
	{
#ifdef _MSC_VER
// 		int totalBytes = _vscprintf(format, argptr) + 1;
// 		ret.Reserve(totalBytes);
// 		vsprintf_s(ret.c_str(), totalBytes, format, argptr);

		int nSize = _vsctprintf(pszFormat, arglist) +1;
		Reserve(nSize);
		//vsprintf_s(m_pBuffer, nSize, pszFormat, arglist);
#else
// 		int totalBytes = vsnprintf(0, 0, format, argptr) + 1;
// 		ret.Reserve(totalBytes);
// 		vsnprintf(ret.c_str(), totalBytes, format, argptr);
#endif
	}

	void CLogger::Close(void)
	{
		::EnterCriticalSection(&m_csLock);
		if (m_hFile != NULL)
		{
			::CloseHandle(m_hFile);
			m_hFile = NULL;
		}
		::LeaveCriticalSection(&m_csLock);
	}

}
