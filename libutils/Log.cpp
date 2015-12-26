#include "stdafx.h"
#include "Log.h"

namespace e
{
	CLogger::CLogger(const TCHAR* pFileName)
	{
		m_hFile = NULL;
		m_nBufferSize = 0;
		m_pBuffer = NULL;
		::InitializeCriticalSection(&m_csLock);

		Open(pFileName);
	}

	CLogger::~CLogger(void)
	{
		Close();
		SafeFree(&m_pBuffer);
		::DeleteCriticalSection(&m_csLock);
	}

	bool CLogger::Reserve(int nCapacity)
	{
		if (nCapacity > m_nBufferSize)
		{
			m_pBuffer = (TCHAR*)realloc(m_pBuffer, nCapacity);
			if (m_pBuffer == NULL) return false;
			assert(m_pBuffer);
			m_nBufferSize = nCapacity;
		}
		return true;
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

	void CLogger::Write(const TCHAR* pPrefix, const TCHAR* pszFormat, va_list arglist)
	{
		int nPreSize = _tcslen(pPrefix);
		int nSize =  _vsctprintf(pszFormat, arglist) + sizeof(TCHAR) * 3;
		if (!Reserve(nSize)) return;
		_vstprintf_s(m_pBuffer, nSize - 2, pszFormat, arglist);
		_tcscat_s(m_pBuffer, nSize, _T("\n"));
		Write(pPrefix, nPreSize);
		Write(m_pBuffer, nSize);
	}

	void CLogger::Write(const TCHAR* pszMsg, int nSize)
	{
		assert(m_hFile);
		DWORD dwWroteBytes;
		WriteFile(m_hFile, pszMsg, nSize, &dwWroteBytes, NULL);
		assert(nSize == dwWroteBytes);
	}

	void CLogger::d(const TCHAR* pFormat, ...)
	{
		::EnterCriticalSection(&m_csLock);
		va_list ap;
		va_start(ap, pFormat);
		Write(_T("debug: "), pFormat, ap);
		va_end(ap);
		::LeaveCriticalSection(&m_csLock);
	}

	void CLogger::i(const TCHAR* pFormat, ...)
	{
		::EnterCriticalSection(&m_csLock);
		va_list ap;
		va_start(ap, pFormat);
		Write(_T("info: "), pFormat, ap);
		va_end(ap);
		::LeaveCriticalSection(&m_csLock);
	}

	void CLogger::w(const TCHAR* pFormat, ...)
	{
		::EnterCriticalSection(&m_csLock);
		va_list ap;
		va_start(ap, pFormat);
		Write(_T("warnning: "), pFormat, ap);
		va_end(ap);
		::LeaveCriticalSection(&m_csLock);
	}

	void CLogger::e(const TCHAR* pFormat, ...)
	{
		::EnterCriticalSection(&m_csLock);
		va_list ap;
		va_start(ap, pFormat);
		Write(_T("error: "), pFormat, ap);
		va_end(ap);
		::LeaveCriticalSection(&m_csLock);
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

	void Log(const TCHAR* pFormat, ...)
	{
		va_list ap;
		va_start(ap, pFormat);
		int nSize = _vsctprintf(pFormat, ap) + sizeof(TCHAR) * 256;
		TCHAR* pBuffer = new TCHAR[nSize];
		_vstprintf_s(pBuffer, nSize, pFormat, ap);
		_tcscat_s(pBuffer, nSize, _T("\n"));
		va_end(ap);
		OutputDebugString(pBuffer);
		delete[] pBuffer;
	}
}
