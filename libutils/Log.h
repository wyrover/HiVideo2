#pragma once
namespace e
{
	class CLogger
	{
	public:
		CLogger(const TCHAR* pFileName);
		virtual ~CLogger(void);
	public:
		void d(const TCHAR* pFormat, ...);
		void i(const TCHAR* pFormat, ...);
		void w(const TCHAR* pFormat, ...);
		void e(const TCHAR* pFormat, ...);
	private:
		bool Open(const TCHAR* pFileName);
		bool Reserve(int nCapacity);
		void Write(const TCHAR* pFormat, va_list arglist);
		void Close(void);
	private:
		HANDLE m_hFile;
		TCHAR* m_pBuffer;
		CRITICAL_SECTION m_csLock;
	};
}

