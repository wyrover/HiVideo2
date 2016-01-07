#pragma once
namespace e
{
	class CImageDumper
	{
	public:
		CImageDumper(void);
		virtual ~CImageDumper(void);
		void Difference(const TCHAR* pFileName, CBitmap* pBG, CBitmap* pFG);
		void Alpha2Text(const TCHAR* pFileName, CBitmap* pAlpha);
	protected:
		void CompareImage(CBitmap* pDest, CBitmap* pSrc0, CBitmap* pSrc1);
		void SaveCompareInfo(const TCHAR* lpFileName, CBitmap* pDiff);
		DWORD ThreadProc(LPVOID lpParam);
		static DWORD WINAPI ThreadProxy(LPVOID lpParam);
	protected:
		bool m_bFinished;
		HANDLE m_hThread;
	};
}