#include "stdafx.h"
#include "VideoMatting.h"
#include "Bitmap.h"
#include "ImageFilter.h"
#include "ColorSpace.h"

namespace e
{
	CVideoMatting::CVideoMatting(void)
		: m_nNumberOfImage(_max_image_count)
	{
		m_nState = STATE_RESET;
		m_pImages = new CBitmap*[m_nNumberOfImage];
		assert(m_pImages);
		for (int i = 0; i < m_nNumberOfImage; i++)
		{
			m_pImages[i] = new CBitmap();
			assert(m_pImages[i]);
		}

		m_pFilter = new CImageFilter();
		assert(m_pFilter);

		m_nThreshold = 15;
		m_nBlockSize = 8;
	}

	CVideoMatting::~CVideoMatting(void)
	{
		Clean();
	}

	CBitmap* CVideoMatting::GetBitmap(ImageType eType) const 
	{
		return m_pImages[eType];
	}

	bool CVideoMatting::InitializeImage(int nWidth, int nHeight, int nBitCount)
	{
		if (!GetBitmap(real_bg_gray)->Create(nWidth, nHeight, 8, NULL))
			return false;
		if (!GetBitmap(real_bg_mask)->Create(nWidth, nHeight, 8, NULL))
			return false;
		if (!GetBitmap(current_gray)->Create(nWidth, nHeight, 8, NULL))
			return false;
		if (!GetBitmap(current_mask)->Create(nWidth, nHeight, 8, NULL))
			return false;
		m_nState &= STATE_MATTING;
		return true;
	}

	void CVideoMatting::SetRealBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		TCHAR szPath[MAX_PATH] = { 0 };
		_stprintf_s(szPath, _T("f:\\bk%u.bmp"), GetTickCount());
		CBitmap::Save(szPath, nWidth, nHeight, nBitCount, pData);

		m_pFilter->Smooth(pData, nSize, nWidth, nHeight, nBitCount);
#ifdef ENABLE_SPLITBLOCK
		m_pFilter->SplitBlock(pData, nSize, nWidth, nHeight, nBitCount, m_nBlockSize);
#endif
		CBitmap* pImage = GetBitmap(real_bg_image);
		pImage->Store(pData, nWidth, nHeight, nBitCount);
		PreprecessBackground();
		m_nState &= STATE_SET_RBG;
	}

	void CVideoMatting::SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		GetBitmap(virtual_bg_image)->Store(pData, nWidth, nHeight, nBitCount);
		m_nState &= STATE_SET_VBG;
	}

	void CVideoMatting::SetThreshold(int nThreshold)
	{
		m_nThreshold = nThreshold;
	}

	void CVideoMatting::Gray(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nBitCount * nWidth);

		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x++)
			{
				int R = p[0], G = p[1], B = p[2];
				p[3] = p[2] = p[1] = p[0] = (R * 76 + G * 150 + B * 30 + 128) >> 8;
				p += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting::Gray(void* pGray, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes0 = WidthBytes(8 * nWidth);
		int nLineBytes1 = WidthBytes(nBitCount * nWidth);
		float lval, aval, bval;
		for (int y = 0; y < nHeight; y++)
		{
			uint8* pDst = (uint8*)pGray + y * nLineBytes0;
			uint8* pSrc = (uint8*)pData + y * nLineBytes1;		
			for (int x = 0; x < nWidth; x++)
			{
				int R = pSrc[0], G = pSrc[1], B = pSrc[2];
				//pDst[x] = (R * 76 + G * 150 + B * 30 + 128) >> 8;
				RGB2LAB(pSrc[0], pSrc[1], pSrc[2], &lval, &aval, &bval);
				pDst[x] = max(0, min(lval * 255 / 100, 255));
				pSrc += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting::PreprecessBackground(void)
	{
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pGray = GetBitmap(real_bg_gray);
		//转化为灰度图
		Gray(pGray->GetBits(), pBG->GetBits(), pBG->Size(), pBG->Width(), pBG->Height(), pBG->BitCount());
	}

	template<class T> T diff(T& a, T & b, int& t)
	{
		return (abs(a - b) < t) ? 0 : 1;
	}

	template<class T> T square(T x)
	{
		return x * x;
	}

	template<class T> T diff(T&r0, T&b0, T&g0, T&r1, T&b1, T&g1, int& t)
	{
		T dx = abs(r0-r1) + abs(b0-b1) + abs(g0 - g1);
		//T dx = sqrt(square(r0 - r1) + square(b0 - b1) + square(g0 - g1));
		return (dx < t) ? 0 : 1;
	}

	//初步计算mask
	void CVideoMatting::CalcMask(void* pMask, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nWidth * nBitCount);

		if (nBitCount == 8)
		{
			uint8* pSrc0 = (uint8*)pBG;
			uint8* pSrc1 = (uint8*)pFG;
			uint8* pDest = (uint8*)pMask;
			for (int y = 0; y < nHeight; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					pDest[x] = diff(pSrc1[x], pSrc0[x], m_nThreshold);
				}
				pSrc0 += nLineBytes;
				pSrc1 += nLineBytes;
				pDest += nLineBytes;
			}
		}
		else
		{
			for (int y = 0; y < nHeight; y++)
			{
				uint8* pSrc0 = (uint8*)pBG + y * nLineBytes;
				uint8* pSrc1 = (uint8*)pFG + y * nLineBytes;
				uint8* pDest = (uint8*)pMask + y * nWidth;
				for (int x = 0; x < nWidth; x++)
				{
					pDest[x] = diff(pSrc0[0], pSrc0[1], pSrc0[2], pSrc1[0], pSrc1[1], pSrc1[2], m_nThreshold);
					pSrc0 += nBitCount >> 3;
					pSrc1 += nBitCount >> 3;
				}
			}
		}
	}

	void CVideoMatting::CalcMatting(void* pData, void* pMask, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes0 = WidthBytes(nWidth * nBitCount);
		int nLineBytes1 = WidthBytes(nWidth * 8);
		for (int y = 0; y < nHeight; y++)
		{
			uint8* pDst = (uint8*)pData+ y * nLineBytes0;
			uint8* pMsk = (uint8*)pMask + y * nLineBytes1;
			for (int x = 0; x < nWidth; x++)
			{
				if (!pMsk[x])
				{
					pDst[0] = 0;
					pDst[1] = 0xff;
					pDst[2] = 0;
					pDst[3] = 0xff;
				}

				pDst += nBitCount >> 3;
			}
		}
	}

	bool CVideoMatting::IsReadyMatting(void) const 
	{
		return (m_nState & STATE_INITIALIZE)
			&& (m_nState & STATE_SET_RBG)
			&& (m_nState & STATE_SET_VBG);
	}

	void CVideoMatting::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//if (!IsReadyMatting()) return;
		GetBitmap(current_image)->Store(pData, nWidth, nHeight, nBitCount);

		m_pFilter->Smooth(pData, nSize, nWidth, nHeight, nBitCount);
#ifdef ENABLE_SPLITBLOCK
		m_pFilter->SplitBlock(pData, nSize, nWidth, nHeight, nBitCount, m_nBlockSize);
#endif

#if 0
		CBitmap* pCurGray = GetBitmap(current_gray);
		Gray(pCurGray->GetBits(), pData, nSize, nWidth, nHeight, nBitCount);

		CBitmap* pBGray = GetBitmap(real_bg_gray);
		CBitmap* pCurMask = GetBitmap(current_mask);
		CalcMask(pCurMask->GetBits(), pBGray->GetBits(), pCurGray->GetBits(), nWidth, nHeight, 8);
		CalcMatting(pData, pCurMask->GetBits(), nWidth, nHeight, nBitCount);
#else
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pMask = GetBitmap(current_mask);
		CalcMask(pMask->GetBits(), pBG->GetBits(), pData, nWidth, nHeight, nBitCount);
		CalcMatting(pData, pMask->GetBits(), nWidth, nHeight, nBitCount);
#endif
	}

	void CVideoMatting::OnSampleSave(const TCHAR* pFileName)
	{
		GetBitmap(current_image)->Save(pFileName);
	}

	void CVideoMatting::Reset(void)
	{
		m_nState = 0;
	}

	void CVideoMatting::Clean(void)
	{
		for (int i = 0; i < m_nNumberOfImage; i++)
		{
			SafeDelete(&m_pImages[i]);
		}
		SafeDelete(&m_pImages);
	}
}