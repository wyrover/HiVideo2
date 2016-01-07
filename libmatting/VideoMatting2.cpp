#include "stdafx.h"
#include "VideoMatting2.h"
#include "Filter.h"
#include "Differen.h"

namespace e
{
	inline int square(int x)
	{
		return x * x;
	}

	inline int rgb_diff(BYTE* p0, BYTE* p1)
	{
		return abs(p0[0] - p1[0]) + abs(p0[1] - p1[1]) + abs(p0[2] - p1[2]);
	}

	inline int rgb_diff(int r0, int b0, int g0, int r1, int b1, int g1)
	{
		return abs(r0 - r1) + abs(b0 - b1) + abs(g0 - g1);
	}

	inline int rgb_dist(BYTE* p0, BYTE* p1)
	{
		return (int)sqrt(square(p0[0] - p1[0]) + square(p0[1] - p1[1]) + square(p0[2] - p1[2]));
	}

	inline int rgb_dist(int r0, int b0, int g0, int r1, int b1, int g1)
	{
		return (int)sqrt(square(r0 - r1) + square(b0 - b1) + square(g0 - g1));
	}

	CVideoMatting2::CVideoMatting2()
		: m_nNumberOfBitmap(_max_count_)
		, m_nBlockSize(4)
		, m_nPreprocOption(preproc_undo)
		, m_nDistanceThreshold(30)
	{
		m_pBitmaps = new CBitmap*[m_nNumberOfBitmap];
		for (int i = 0; i < m_nNumberOfBitmap; i++)
		{
			m_pBitmaps[i] = new CBitmap();
			assert(m_pBitmaps[i]);
		}

		m_pFilter = new CImageFilter();
		assert(m_pFilter);

		m_pDiffer = new CDifferen();
		assert(m_pDiffer);

		//SetPreprocOption(preproc_smooth, true);
		SetPreprocOption(preproc_block, true);
		SetPreprocOption(preproc_noisc, true);
	}

	CVideoMatting2::~CVideoMatting2()
	{
		SafeDelete(&m_pDiffer);
		SafeDelete(&m_pFilter);
		for (int i = 0; i < m_nNumberOfBitmap; i++)
		{
			SafeDelete(&m_pBitmaps[i]);
		}
		SafeDeleteArray(&m_pBitmaps);
	}

	CBitmap* CVideoMatting2::GetBitmap(ImageType2 eType) const
	{
		return m_pBitmaps[eType];
	}

	void CVideoMatting2::SetPreprocOption(PreprocOption eOption, bool bAdd)
	{
		if (bAdd)
			m_nPreprocOption |= eOption;
		else
			m_nPreprocOption &= ~eOption;
	}

	void CVideoMatting2::SetMattingMode(MattingMode eMode)
	{
		m_nMattingMode = eMode;
	}

	void CVideoMatting2::SetMattingThreshold(int nIndex, int nThreshold)
	{
		if (nIndex == 0)
			m_nDistanceThreshold = nThreshold;
		else if (nIndex == 1)
			m_nNoiscThreshold = nThreshold;

		if (nIndex == 0)
			m_pDiffer->SetThreshold(nThreshold);
	}

	bool CVideoMatting2::Initialize(int nWidth, int nHeight, int nBitCount)
	{
		if (!GetBitmap(bg_block)->Create(nWidth, nHeight, nBitCount, NULL, true)) return false;
		if (!GetBitmap(background)->Create(nWidth, nHeight, nBitCount, NULL, true)) return false;
		if (!GetBitmap(foreground)->Create(nWidth, nHeight, nBitCount, NULL, true)) return false;

		if (!GetBitmap(fg_gray)->Create(nWidth, nHeight, 8, NULL, true)) return false;
		if (!GetBitmap(trimap)->Create(nWidth, nHeight, 8, NULL, true)) return false;
		if (!GetBitmap(alpha)->Create(nWidth, nHeight, 8, NULL, true)) return false;

		nWidth /= m_nBlockSize;
		nHeight /= m_nBlockSize;
		if (!GetBitmap(graph)->Create(nWidth, nHeight, 8, NULL, true)) return false;
		if (!GetBitmap(edge)->Create(nWidth, nHeight, 8, NULL, true)) return false;
		return true;
	}

	void CVideoMatting2::PreprocessImage(CBitmap* pImage)
	{
		void* pData = pImage->GetBits();
		int nSize = pImage->Size();
		int nWidth = pImage->Width();
		int nHeight = pImage->Height();
		int nBitCount = pImage->BitCount();
		//平滑去噪
		if (m_nPreprocOption & preproc_smooth)
		{
			m_pFilter->Smooth(pData, nSize, nWidth, nHeight, nBitCount);
		}
		//转成灰度图，保存在alpha通道中
		if (m_nPreprocOption & preproc_gray)
		{
			m_pFilter->ConvertGray(pData, nSize, nWidth, nHeight, nBitCount);
		}
		//分割成块
		if (m_nPreprocOption & preproc_block)
		{
			m_pFilter->SplitBlock(pData, nSize, nWidth, nHeight, nBitCount, m_nBlockSize);
		}
	}

	void CVideoMatting2::SetBackground(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
#ifndef _DEBUG 
		CBitmap::Save(_T("f:\\bg.bmp"), nWidth, nHeight, nBitCount, pData);
#endif
		CBitmap* pBG = GetBitmap(background);
		pBG->Store(pData, nWidth, nHeight, nBitCount);
		CBitmap* pBB = GetBitmap(bg_block);
		pBB->Store(pData, nWidth, nHeight, nBitCount);
		//对背景图片进行预处理
		PreprocessImage(pBB);
	}

	void CVideoMatting2::SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{

	}

	void CVideoMatting2::RemoveNoiscEnable(bool bEnable)
	{
		SetPreprocOption(preproc_noisc, bEnable);
	}

	void CVideoMatting2::RemoveNoisc(CBitmap* pGraph)
	{
		if (m_nPreprocOption & preproc_noisc)
			m_pFilter->RemoveNoise(pGraph);
	}

	void CVideoMatting2::CalcEdge(CBitmap* pEdge, CBitmap* pGraph)
	{
		m_pFilter->CalcEdge(pGraph, pEdge);
	}

	void CVideoMatting2::CalcMerge(CBitmap* pGraph, CBitmap* pEdge)
	{
		m_pFilter->CalcMerge(pGraph, pEdge);
	}

	void CVideoMatting2::CalcMerge(CBitmap* pTrimap, CBitmap* pGraph, CBitmap* pEdge)
	{
		m_pFilter->CalcMerge(pTrimap, pGraph, pEdge);
	}

	void CVideoMatting2::MakeGraph(CBitmap* pGraph, CBitmap* pBG, CBitmap* pFG)
	{
		//calculate color distance from BG & FG
		int nWidth = pBG->Width();
		int nHeight = pBG->Height();
		int nPixelSize = pBG->PixelSize();
		for (int y = 0; y < nHeight; y += m_nBlockSize)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = pFG->GetBits(0, y);
			BYTE* pG = pGraph->GetBits(0, y / m_nBlockSize);
			for (int x = 0; x < nWidth; x += m_nBlockSize)
			{
				int dx = rgb_dist(pB[0], pB[1], pB[2], pF[0], pF[1], pF[2]);
				*pG++ = (dx <= m_nDistanceThreshold) ? 0 : 255;
				pB += m_nBlockSize*nPixelSize;
				pF += m_nBlockSize*nPixelSize;
			}
		}
	}

	inline void SetPixel(int x, int y, int c, BYTE* p, int w, int h, int bits, int line)
	{
		x = (x<0) ? 0 : ((x>w) ? w : x);
		y = (y<0) ? 0 : ((y>h) ? h : y);
		*(p + y * line + x * (bits >> 3)) = c;
	}

	// 基于Bresenham算法画填充圆
	void CVideoMatting2::FillRound(int x, int y, int r, BYTE* pData, int nWidth, int nHeight, int nBitCount)
	{	
		int tx = 0, ty = r, d = 3 - 2 * r, color = 128, w = nWidth - 1, h = nHeight - 1;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		while (tx < ty)
		{
			for (int i = x - ty; i <= x + ty; i++)
			{
				SetPixel(i, y - tx, color, pData, w, h, nBitCount, nLineSize);
				if (tx != 0)
				{
					SetPixel(i, y + tx, color, pData, w, h, nBitCount, nLineSize);
				}
			}

			if (d < 0)
			{
				d += 4 * tx + 6;
			}
			else
			{
				for (int i = x - tx; i <= x + tx; i++)
				{
					SetPixel(i, y - ty, color, pData, w, h, nBitCount, nLineSize);
					SetPixel(i, y + ty, color, pData, w, h, nBitCount, nLineSize);
				}

				d += 4 * (tx - ty) + 10, ty--;
			}

			tx++;
		}

		if (tx == ty)
		{
			for (int i = x - ty; i <= x + ty; i++)
			{
				SetPixel(i, y - tx, color, pData, w, h, nBitCount, nLineSize);
				SetPixel(i, y + tx, color, pData, w, h, nBitCount, nLineSize);
			}
		}
	}

	int CalcDist(int x, int y, BYTE* p, int w, int h, int bits, int line)
	{
		return 0;
	}

	void CVideoMatting2::MakeAlpha(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, void* pFG)
	{
		int nWidth = pBG->Width(); 
		int nHeight = pBG->Height();
		int nBitCount = pBG->BitCount();
		int nLineSize = WidthBytes(nWidth*nBitCount);

		pAlpha->Store(pTrimap);
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = (BYTE*)pFG + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				if (*pA == 128)
				{
					int nAlpha = rgb_dist(pB[0], pB[1], pB[2], pF[0], pF[1], pF[2]);
					nAlpha = max(0, min(nAlpha, 255));
					*pA = (nAlpha < m_nNoiscThreshold) ? 0 : 255;
					//*pA = nAlpha;
				}
				else
				{
				//	*pA = 0;
				}

				pA++;
				pB += nBitCount >> 3;
				pF += nBitCount >> 3;
			}
		}
	}

	inline void CalcMatting(BYTE* pF, BYTE* pV, BYTE a)
	{
		pF[0] = a * pF[0] / 255 + (255 - a)*pV[0] / 255;
		pF[1] = a * pF[1] / 255 + (255 - a)*pV[1] / 255;
		pF[2] = a * pF[2] / 255 + (255 - a)*pV[2] / 255;
	}

	void CVideoMatting2::MakeMatting(void* pData, CBitmap* pAlpha, CBitmap* pVG)
	{
		int nWidth = pAlpha->Width();
		int nHeight = pAlpha->Height();
		int nBitCount = 32;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		int nColor = 0xff00ff00;
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pF = (BYTE*)pData + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				CalcMatting(pF, (BYTE*)&nColor, *pA++);
				pF += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting2::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
 		m_pFilter->ConvertGray(pData, nSize, nWidth, nHeight, nBitCount);
		m_pFilter->CalcEdge(pData, nSize, nWidth, nHeight, nBitCount);
		return;
		CBitmap* pBB = GetBitmap(bg_block);
		CBitmap* pBG = GetBitmap(background);
		CBitmap* pFG = GetBitmap(foreground);
#if 0
		m_pDiffer->Sub(pData, pBG->GetBits(), pData, nWidth, nHeight, nBitCount);
		return;
#endif
		pFG->Store(pData, nWidth, nHeight, nBitCount);
		//pFG->Save(_T("f:\\fg.bmp"));
		//preprocess
		PreprocessImage(pFG);
		//make graph
		CBitmap* pGraph = GetBitmap(graph);
		MakeGraph(pGraph, pBB, pFG);
		pGraph->Save(_T("f:\\graph0.bmp"));
		//remove noisc
		RemoveNoisc(pGraph);
		pGraph->Save(_T("f:\\graph1.bmp"));
		//make edge
		CBitmap* pEdge = GetBitmap(edge);
		CalcEdge(pEdge, pGraph);
		pEdge->Save(_T("f:\\edge.bmp"));
		//make trimap
		CBitmap* pTrimap = GetBitmap(trimap);
		CalcMerge(pTrimap, pGraph, pEdge);
		pTrimap->Save(_T("f:\\trimap.bmp"));
		//make alpha
		CBitmap* pAlpha = GetBitmap(alpha);
		MakeAlpha(pAlpha, pTrimap, pBG, pData);
		pAlpha->Save(_T("f:\\alpha.bmp"));

		MakeMatting(pData, pAlpha, NULL);
	}

	void CVideoMatting2::OnSampleSave(TCHAR* pFileName)
	{

	}

	void CVideoMatting2::Reset(void)
	{

	}
}
