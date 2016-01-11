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
		, m_nMattingMode(SimpleMatting)
		, m_nDistanceThreshold(20)
		, m_nNoiscThreshold(25)
		, m_nAlphaThreshold(25)
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
		else if (nIndex == 2)
			m_nAlphaThreshold = nThreshold;

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

	void CVideoMatting2::SetVirtualBackground(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
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

	inline int around_diff(int x, int y, BYTE* p0, int w, int h, int bpp, int line, int t)
	{
		BYTE* p1 = (y > 0) ? p0 - line : p0;
		BYTE* p2 = (y < h - 1) ? p0 + line : p0;
		BYTE* p3 = (x>0) ? p0 - bpp : p0;
		BYTE* p4 = (x < w - 1) ? p0 + bpp : p0;

// 		BYTE* p5 = (x > 0) ? p1 - bpp : p1;
// 		BYTE* p6 = (x < w - 1) ? p1 + bpp : p1;
// 		BYTE* p7 = (x > 0) ? p2 - bpp : p2;
// 		BYTE* p8 = (x < w - 1) ? p2 + bpp : p2;

		int dist = 0, sum = 0, count = 0;
		dist = rgb_dist(p0, p1);
		if (dist <= t) { sum += dist; count++; }
		dist = rgb_dist(p0, p2);
		if (dist <= t) { sum += dist; count++; }
		dist = rgb_dist(p0, p3);
		if (dist <= t) { sum += dist; count++; }
		dist = rgb_dist(p0, p4);
		if (dist <= t) { sum += dist; count++; }
		return count > 0 ? sum / count : 0;
	}

	void CVideoMatting2::MakeAlphaSimple(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, void* pFG)
	{
		int nWidth = pBG->Width();
		int nHeight = pBG->Height();
		int nBitCount = pBG->BitCount();
		int nPixelSize = pBG->PixelSize();
		int nLineSize = WidthBytes(nWidth*nBitCount);
		pAlpha->Store(pTrimap);
		//第1趟，初步分出边界
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = (BYTE*)pFG + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				if (*pA == 128)
				{
					int dx = rgb_dist(pB, pF);
					dx = max(0, min(dx, 255));
					*pA = (dx < m_nAlphaThreshold) ? 0 : dx;
				}

				pA++;
				pB += nPixelSize;
				pF += nPixelSize;
			}
		}
	}

	inline int CalcSampleAlpha(int x, int y, BYTE* pBG, BYTE* pFG, int w, int h, int bpp, int line, int dt)
	{
		int diff = rgb_diff(pBG, pFG);
		if (diff <= dt) return 0;
		int dx1 = around_diff(x, y, pBG, w, h, bpp, line, diff - dt > 30 ? dt : diff);
		int dx2 = around_diff(x, y, pFG, w, h, bpp, line, diff - dt > 30 ? dt : diff);
		if (dx1 == dx2) return 128;
		if (dx1 < dx2) return max(0, min(dx1 * 255 / dx2, 255));
		if (dx1 > dx2) return max(0, min(dx2 * 255 / dx1, 255));
		return 0;
	}

	void CVideoMatting2::MakeAlphaNormal(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, void* pFG)
	{
		int nWidth = pBG->Width();
		int nHeight = pBG->Height();
		int nBitCount = pBG->BitCount();
		int nPixelSize = pBG->PixelSize();
		int nLineSize = WidthBytes(nWidth*nBitCount);
		pAlpha->Store(pTrimap);
		//第1趟，初步分出边界
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = (BYTE*)pFG + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				if (*pA == 128)
				{
					// 此处可以分开头发
					int dx = rgb_dist(pB, pF);
					dx = max(0, min(dx, 255));
					*pA = (dx < m_nNoiscThreshold) ? 0 : dx;
				}
				else
				{
					//	*pA = 0;
				}

				pA++;
				pB += nPixelSize;
				pF += nPixelSize;
			}
		}
		return;
// 		//第二步，精准定位边界
// 		for (int y = 0; y < nHeight-1; y++)
// 		{
// 			BYTE* p0 = pAlpha->GetBits(0, y);
// 			BYTE* p1 = p0 + nWidth;
// 			for (int x = 0; x < nWidth-1; x++)
// 			{
// 				int dx = abs(*p0 - *(p1 + nPixelSize)) + abs(*p1 - *(p0 + nPixelSize));
// 				*p0 = max(0, min(dx, 255));
// 				p0++; p1++;
// 			}
// 		}

		//第3步，对边界进行详细处理
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = (BYTE*)pFG + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				if (*pA == 255)
				{
					*pA = CalcSampleAlpha(x, y, pB, pF, nWidth, nHeight, nPixelSize, nLineSize, m_nAlphaThreshold);
				}

				pA++;
				pB += nPixelSize;
				pF += nPixelSize;
			}
		}
	}

	inline void Blend(BYTE* pF, BYTE* pV, BYTE a)
	{
		pF[0] = a * pF[0] / 255 + (255 - a)*pV[0] / 255;
		pF[1] = a * pF[1] / 255 + (255 - a)*pV[1] / 255;
		pF[2] = a * pF[2] / 255 + (255 - a)*pV[2] / 255;
	}

	void CVideoMatting2::MakeMatting(void* pData, int nWidth, int nHeight, int nBitCount, CBitmap* pAlpha, CBitmap* pVG)
	{
		int nPixelSize = nBitCount >> 3;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		int G = 0xff00ff00, R = 0xffffff00;
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pF = (BYTE*)pData + y * nLineSize;
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int nAlpha = *pA++;
				if (nAlpha == 0x00)
				{
					Blend(pF, (BYTE*)&G, nAlpha);
				}
				else if (nAlpha != 0xff)
				{
					Blend(pF, (BYTE*)&R, nAlpha);
				}
				pF += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting2::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
// 		m_pFilter->ConvertGray(pData, nSize, nWidth, nHeight, nBitCount);
// 		m_pFilter->CalcEdge(pData, nSize, nWidth, nHeight, nBitCount);
// 		return;
		CBitmap* pBB = GetBitmap(bg_block);
		CBitmap* pBG = GetBitmap(background);
		CBitmap* pFG = GetBitmap(foreground);
#if 0
		m_pDiffer->SampleWeight(pData, nWidth, nHeight, nBitCount, m_nDistanceThreshold);
		return;
#endif
		pFG->Store(pData, nWidth, nHeight, nBitCount);
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
		if (m_nMattingMode == SimpleMatting)
			MakeAlphaSimple(pAlpha, pTrimap, pBG, pData);
		else
			MakeAlphaNormal(pAlpha, pTrimap, pBG, pData);
		pAlpha->Save(_T("f:\\alpha.bmp"));
		//make matting
		MakeMatting(pData, nWidth, nHeight, nBitCount, pAlpha, NULL);
	}

	void CVideoMatting2::OnSampleSave(TCHAR* pFileName)
	{
		CBitmap* pFG = GetBitmap(foreground);
		pFG->Save(pFileName);
	}

	void CVideoMatting2::Reset(void)
	{

	}
}
