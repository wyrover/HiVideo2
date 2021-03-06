#pragma once
#include "IVideoDevice.h"

namespace DuiLib
{
#define _THREAD_SAFE 1

	class CVideoUI : public CControlUI
	{
	public:
		CVideoUI();
		virtual ~CVideoUI();
		LPCTSTR GetClass(void) const;
		LPVOID GetInterface(LPCTSTR pstrName);
	public:
		bool Init(int nWidth, int nHeight, int nBitCount);
		void DoRenderSample(void* pData, int nWidth, int nHeight, int nBitCount);
		void DoRenderSample(DWORD dwColor);
		virtual void DoEvent(TEventUI& event);
		virtual void DoPaint(HDC hDC, const RECT& rcPaint);
	private:
		BYTE*		m_pBits;
		DWORD	m_dwSize;
		int			m_nWidth;
		int			m_nHeight;
		HBITMAP m_hBitmap;
		RECT		m_rcBitmap;
		RECT		m_rcCorners;
#if _THREAD_SAFE
		CRITICAL_SECTION m_csLock;
#endif
	};
}
