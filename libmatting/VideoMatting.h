#pragma once

namespace e
{
	class CBitmap;
	class CVideoMatting
	{
	public:
		CVideoMatting(void);
		virtual ~CVideoMatting(void);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		CBitmap* GetBitmap(int nIndex);
		void SwapBitmap(void);
	protected:
		int m_nNumberOfBitmap;
		CBitmap** m_pBitmaps;
	};
}
