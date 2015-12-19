#include "stdafx.h"
#include "VideoMatting.h"
#include "Bitmap.h"

namespace e
{
	CVideoMatting::CVideoMatting()
	{
		m_nNumberOfBitmap = 2;
		m_pBitmaps = new CBitmap*[m_nNumberOfBitmap];
		for (int i = 0; i < m_nNumberOfBitmap; i++)
		{
			m_pBitmaps[i] = new CBitmap();
		}
	}

	CVideoMatting::~CVideoMatting()
	{
		if (m_pBitmaps != NULL)
		{
			for (int i = 0; i < m_nNumberOfBitmap; i++)
			{
				if (m_pBitmaps[i] != NULL)
				{
					delete m_pBitmaps[i];
					m_pBitmaps[i] = NULL;
				}
			}

			delete[] m_pBitmaps;
		}
	}

	CBitmap* CVideoMatting::GetBitmap(int nIndex)
	{
		return m_pBitmaps[nIndex];
	}

	void CVideoMatting::SwapBitmap(void)
	{
		CBitmap* pTemp = m_pBitmaps[0];
		m_pBitmaps[0] = m_pBitmaps[1];
		m_pBitmaps[1] = pTemp;
	}

#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
	template<class T> T GetValue(T& a, T & b)
	{
		T dx = abs(a - b);
		return (dx < 20) ? 0 : b;
	}
	void CVideoMatting::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		CBitmap* pBitmap = GetBitmap(0);
		if (pBitmap->GetBits() == NULL)
		{
			pBitmap->Create(nWidth, nHeight, nBitCount, pData);
			return;
		}

		CBitmap* pTemp = GetBitmap(1);
		pTemp->Create(nWidth, nHeight, nBitCount, pData);

		BYTE* pSrc = pBitmap->GetBits();
		BYTE* pDst = (BYTE*)pData;
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
// 				pDst[0] = pSrc[0] - pDst[0] + 128;
// 				pDst[1] = pSrc[1] - pDst[1] + 128;
// 				pDst[2] = pSrc[2] - pDst[2] + 128;
// 				pDst[3] = pSrc[3] - pDst[3] + 128;

				pDst[0] = GetValue(pSrc[0], pDst[0]);
				pDst[1] = GetValue(pSrc[1], pDst[1]);
				pDst[2] = GetValue(pSrc[2], pDst[2]);
				pDst[3] = GetValue(pSrc[3], pDst[3]);

				pDst += 4;
				pSrc += 4;
			}
		}

		SwapBitmap();
	}
}