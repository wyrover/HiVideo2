#include "stdafx.h"
#include "Filter.h"
#include "ImageBlur.h"
#include "Region.h"

namespace e
{
	template<class T> T square(T x)
	{
		return x * x;
	}
	/* normalize mask so it integrates to one */
	void normalize(float* mask, int len) 
	{
		float sum = 0;
		for (int i = 1; i < len; i++) 
		{
			sum += fabs(mask[i]);
		}
		sum = 2 * sum + fabs(mask[0]);
		for (int i = 0; i < len; i++) 
		{
			mask[i] /= sum;
		}
	}

	CImageFilter::CImageFilter(void)
	{
		m_nRadius = 2;
		m_fSigma = 1.5f;
		m_pKernals = 0;
		m_pTemp = NULL;
		CalcKernals(m_fSigma);

		m_pBlur = new CImageBlur();
		assert(m_pBlur);

		m_pGraphTemp = new CBitmap();
		assert(m_pGraphTemp);

		m_pRegion = NULL;
	}

	CImageFilter::~CImageFilter(void)
	{
		SafeFree(&m_pTemp);
		SafeFree(&m_pKernals);
		SafeDelete(&m_pBlur);
		SafeDelete(&m_pGraphTemp);
		SafeDelete(&m_pRegion);
	}

	bool CImageFilter::Initialize(int nWidth, int nHeight, int nBitCount)
	{
		int nSize = WidthBytes(nBitCount * nWidth) * nHeight;
		m_pTemp = realloc(m_pTemp, nSize);
		return m_pTemp != NULL;
	}

	void CImageFilter::CalcKernals(float fSigma)
	{
		fSigma = max(fSigma, 0.01f);
		int len = (int)ceil(fSigma * m_nRadius * 2) + 1;
		m_pKernals = (float*)realloc(m_pKernals, len * sizeof(float));
		for (int i = 0; i < len; i++)
		{
			m_pKernals[i] = exp(-0.5*square(i / fSigma));
		}
		normalize(m_pKernals, len);
	}

	inline void SetPixel(int val, uint8* p, int x, int y, int step, int bpp)
	{
		p[y * step + x * bpp] = val;
	}

	void CImageFilter::Convolve(void* pSrc, void* pDst, int nWidth, int nHeight, int nBitCount)
	{
		int bpp = nBitCount >> 3;
		int nLength = m_nRadius * 2 + 1;
		int nLineBytes0 = WidthBytes(nWidth * nBitCount);
		int nLineBytes1 = WidthBytes(nHeight * nBitCount);

		uint8* pR1 = (uint8*)pDst;
		uint8* pG1 = pR1 + 1;
		uint8* pB1 = pG1 + 1;

		for (int y = 1; y < nHeight-1; y++)
		{
			uint8* pR0 = (uint8*)pSrc + y * nLineBytes0;
			uint8* pG0 = pR0 + 1;
			uint8* pB0 = pG0 + 1;

			for (int x = 1; x < nWidth-1; x++)
			{
				float fSumR = m_pKernals[0] * pR0[x*bpp];
				float fSumG = m_pKernals[0] * pG0[x*bpp];
				float fSumB = m_pKernals[0] * pB0[x*bpp];

				for (int i = 1; i < nLength; i++)
				{
					fSumR += m_pKernals[i] * (pR0[(x - i)*bpp] + pR0[(x + i)*bpp]);
					fSumG += m_pKernals[i] * (pG0[(x - i)*bpp] + pG0[(x + i)*bpp]);
					fSumB += m_pKernals[i] * (pB0[(x - i)*bpp] + pB0[(x + i)*bpp]);
				}

				SetPixel(fSumR, pR1, y, x, nLineBytes1, bpp);
				SetPixel(fSumG, pG1, y, x, nLineBytes1, bpp);
				SetPixel(fSumB, pB1, y, x, nLineBytes1, bpp);
			}
		}
	}

	void CImageFilter::Smooth(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
#if 0
		if (m_pTemp == NULL)
		{
			Initialize(nHeight, nWidth, nBitCount);
		}

		Convolve(pData, m_pTemp, nWidth, nHeight, nBitCount);
		Convolve(m_pTemp, pData, nHeight, nWidth, nBitCount);
#else
		m_pBlur->OnSampleProc(pData, nSize, nWidth, nHeight, nBitCount);
#endif
	}

	void CImageFilter::GetBlock(void* pData, int nBitCount, int nLineBytes, int nBlockSize)
	{
		int nSum[3] = { 0 };
		for (int y = 0; y < nBlockSize; y++)
		{
			uint8* pSrc = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				nSum[0] += pSrc[0];
				nSum[1] += pSrc[1];
				nSum[2] += pSrc[2];
				pSrc += nBitCount >> 3;
			}
		}

		nSum[0] /= (nBlockSize * nBlockSize);
		nSum[1] /= (nBlockSize * nBlockSize);
		nSum[2] /= (nBlockSize * nBlockSize);
		for (int y = 0; y < nBlockSize; y++)
		{
			uint8* pSrc = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				pSrc[0] = nSum[0];
				pSrc[1] = nSum[1];
				pSrc[2] = nSum[2];
				pSrc += nBitCount >> 3;
			}
		}
	}

	void CImageFilter::SplitBlock(void* pData, int nSize, int nWidth, int nHeight, int nBitCount, int nBlockSize)
	{
		int nLineBytes = WidthBytes(nBitCount * nWidth);
		for (int y = 0; y < nHeight; y += nBlockSize)
		{
			uint8* pSrc = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x += nBlockSize)
			{
				GetBlock(pSrc, nBitCount, nLineBytes, nBlockSize);
				pSrc += (nBlockSize*(nBitCount >> 3));
			}
		}
	}

	void CImageFilter::ConvertGray(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		assert(nBitCount == 32);
		int nLineBytes = WidthBytes(nBitCount * nWidth);
		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x++)
			{
				int R = p[0], G = p[1], B = p[2];
				p[0] = p[1] = p[2]  = (R * 76 + G * 150 + B * 30 + 128) >> 8;
				p += nBitCount >> 3;
			}
		}
	}

	void CImageFilter::ConvertGray(void* pGray, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
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
				pDst[x] = (R * 76 + G * 150 + B * 30 + 128) >> 8;
				pSrc += nBitCount >> 3;
			}
		}
	}

	void CImageFilter::ConvertGray(CBitmap* pImage)
	{
		assert(pImage->BitCount() == 32);
		int nWidth = pImage->Width();
		int nHeight = pImage->Height();
		int nPixelSize = pImage->BitCount() / 8;

		for (int y = 0; y < nHeight; y++)
		{
			uint8* pSrc = pImage->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				//gray data replace alpha channel
				pSrc[3] = (pSrc[2] * 76 + pSrc[1] * 150 + pSrc[0] * 30 + 128) >> 8;
				pSrc += nPixelSize;
			}
		}
	}

	void CImageFilter::Block2Graph(CBitmap* pGraph, CBitmap* pBlock, int nBlockSize)
	{
		int nWidth = pBlock->Width();
		int nHeight = pBlock->Height();
		int nBitCount = pBlock->BitCount();

		for (int y = 0; y < nHeight; y+=nBlockSize)
		{
			uint8* pSrc = pBlock->GetBits(0, y);
			uint8* pDst = pGraph->GetBits(0, y / nBlockSize);
			for (int x = 0; x < nWidth; x+=nBlockSize)
			{
				*pDst++ = pSrc[0] > 0 ? 255 : 0;
				pSrc += nBlockSize*(nBitCount >> 3);
			}
		}
	}

	void CImageFilter::SetBlock(int nValue, void* pData, int nBitCount, int nLineBytes, int nBlockSize)
	{
		for (int y = 0; y < nBlockSize; y++)
		{
			uint8* pSrc = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				pSrc[0] = pSrc[1] = pSrc[2] = nValue;
				pSrc += nBitCount >> 3;
			}
		}
	}

	void CImageFilter::Graph2Block(CBitmap* pBlock, CBitmap* pGraph, int nBlockSize)
	{
		int nWidth = pBlock->Width();
		int nHeight = pBlock->Height();
		int nBitCount = pBlock->BitCount();
		int nLineBytes = WidthBytes(nWidth*nBitCount);

		for (int y = 0; y < nHeight; y += nBlockSize)
		{
			uint8* pSrc = pBlock->GetBits(0, y);
			uint8* pGra = pGraph->GetBits(0, y / nBlockSize);
			for (int x = 0; x < nWidth; x += nBlockSize)
			{
				SetBlock(*pGra++, pSrc, nBitCount, nLineBytes, nBlockSize);
				pSrc += nBlockSize*(nBitCount >> 3);
			}
		}
	}

	//腐蚀黑点(消去黑点)
	void CImageFilter::Erosion(CBitmap* pGraph, int nDirection, bool bBlackPixel)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nBitCount = pGraph->BitCount();
		m_pGraphTemp->Store(pGraph);
		int nWipeValue = bBlackPixel ? 0 : 255;//remove pixel
		int nKeepValue = bBlackPixel ? 255 : 0;
	
		//水平方向
		if (nDirection & 0x01)
		{
			for (int y = 0; y < nHeight; y++)
			{
				BYTE* pSrc = m_pGraphTemp->GetBits(0, y);
				BYTE* pDst = pGraph->GetBits(0, y);
				for (int x = 0; x < nWidth; x++)
				{
					BYTE* pSrc1 = x > 0 ? pSrc - 1 : pSrc;
					BYTE* pSrc2 = x < nWidth - 1 ? pSrc + 1 : pSrc;
					//相邻有一个是保留值
					if (*pSrc1 == nKeepValue || *pSrc2 == nKeepValue)
						*pDst = nKeepValue;
					else
						*pDst = nWipeValue;

					pSrc++;
					pDst++;
				}
			}
		}

		//垂直方向
		if (nDirection & 0x02)
		{
			int nLineBytes = WidthBytes(nWidth*nBitCount);
			for (int x = 0; x < nWidth; x++)
			{
				BYTE* pSrc = m_pGraphTemp->GetBits(x, 0);
				BYTE* pDst = pGraph->GetBits(x, 0);
				for (int y = 0; y < nHeight; y++)
				{
					BYTE* pSrc1 = y > 0 ? pSrc - nLineBytes : pSrc;
					BYTE* pSrc2 = y < nHeight - 1 ? pSrc + nLineBytes : pSrc;
					//相邻有一个是保留值
					if (*pSrc1 == nKeepValue || *pSrc2 == nKeepValue)
						*pDst = nKeepValue;
					else
						*pDst = nWipeValue;

					pSrc += nLineBytes;
					pDst += nLineBytes;
				}
			}
		}
	}

	//膨胀黑点
	void CImageFilter::Dilation(CBitmap* pGraph, int nDirection, bool bBlackPixel)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nBitCount = pGraph->BitCount();
		m_pGraphTemp->Store(pGraph);
		int nWipeValue = bBlackPixel ? 255 : 0;
		int nKeepValue = bBlackPixel ? 0 : 255;

		//水平方向
		if (nDirection & 0x01)
		{
			for (int y = 0; y < nHeight; y++)
			{
				BYTE* pSrc = m_pGraphTemp->GetBits(0, y);
				BYTE* pDst = pGraph->GetBits(0, y);
				for (int x = 0; x < nWidth; x++)
				{
					BYTE* pSrc1 = x > 0 ? pSrc - 1 : pSrc;
					BYTE* pSrc2 = x < nWidth - 1 ? pSrc + 1 : pSrc;

					if (*pSrc1 == nKeepValue || *pSrc2 == nKeepValue)
						*pDst = nKeepValue;
					else
						*pDst = nWipeValue;
					
					pSrc++;
					pDst++;
				}
			}
		}

		//水平方向
		if (nDirection & 0x02)
		{
			int nLineBytes = WidthBytes(nWidth*nBitCount);
			for (int x = 0; x < nWidth; x++)
			{
				BYTE* pSrc = m_pGraphTemp->GetBits(x, 0);
				BYTE* pDst = pGraph->GetBits(x, 0);
				for (int y = 0; y < nHeight; y++)
				{
					BYTE* pSrc1 = y > 0 ? pSrc - nLineBytes : pSrc;
					BYTE* pSrc2 = y < nHeight - 1 ? pSrc + nLineBytes : pSrc;

					if (*pSrc1 == nKeepValue || *pSrc2 == nKeepValue)
						*pDst = nKeepValue;
					else
						*pDst = nWipeValue;

					pSrc += nLineBytes;
					pDst += nLineBytes;
				}
			}
		}
	}

	void CImageFilter::CalcEdge(CBitmap* pGraph, CBitmap* pEdge)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nLineSize = pGraph->LineSize();
		int nPixelSize = pGraph->PixelSize();
		//laplacian算子
		for (int y = 0; y < nHeight; y++)
		{
			uint8* p1 = pGraph->GetBits(0, y);
			uint8* p0 = y > 0 ? p1 - nLineSize : p1;
			uint8* p2 = y < nHeight - 1 ? p1 + nLineSize : p1;
			uint8* p3 = pEdge->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				uint8* p4 = x > 0 ? p1 - 1 : p1;
				uint8* p5 = x < nWidth - 1 ? p1 + 1 : p1;
				int gx = abs((*p4 + *p5 + *p0 + *p2) - 4 * (*p1));
				*p3++ = max(0, min(gx, 255)) & *p1;
				p0 += nPixelSize;
				p1 += nPixelSize;
				p2 += nPixelSize;
			}
		}
	}

	void CImageFilter::CalcEdge(void* pGray, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nPixelSize = nBitCount >> 3;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		for (int y = 1; y < nHeight-1; y++)
		{
			BYTE* p0 = (BYTE*)pGray + y * nLineSize;
			BYTE* p1 = p0 - nLineSize;
			BYTE* p2 = p0 + nLineSize;
			for (int x = 1; x < nWidth-1; x++)
			{
				BYTE* p3 = p0 - nPixelSize;
				BYTE* p4 = p0 + nPixelSize;

// 				BYTE* p5 = p1 - nPixelSize;
// 				BYTE* p6 = p1 + nPixelSize;
// 
// 				BYTE* p7 = p2 - nPixelSize;
// 				BYTE* p8 = p2 + nPixelSize;
// 				int dx = abs((*p1 + *p2 + *p3 + *p4 + *p5 + *p6 + *p7 + *p8) - 8 * (*p0));

				int dx = abs((*p3 + *p4 + *p1 + *p2) - 4 * (*p0));
				p1[0] = p1[1] = p1[2] = max(0, min(dx, 255));

				p0 += nPixelSize;
				p1 += nPixelSize;	
				p2 += nPixelSize;
			}
		}
		return;

		for (int y = 0; y < nHeight - 1; y++)
		{
			BYTE* p1 = (BYTE*)pGray + y * nLineSize;
			BYTE* p2 = p1 + nLineSize;
			for (int x = 0; x < nWidth - 1; x++)
			{
				int dx = abs(*p1 - *(p2 + nPixelSize)) + abs(*p2 - *(p1 + nPixelSize));
				p1[0] = p1[1] = p1[2] = max(0, min(dx, 255));
				p1 += nPixelSize;
				p2 += nPixelSize;
			}
		}
	}

	void CImageFilter::CalcEdge(void* pEdge, void* pGray, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nPixelSize = nBitCount >> 3;
		int nLineSize = WidthBytes(nWidth*nBitCount);
		//---------------------------------------------------------------------------------------------
// 		for (int y = 0; y < nHeight; y++)
// 		{
// 			BYTE* p1 = (BYTE*)pGray + y * nLineSize;
// 			BYTE* p0 = y > 0 ? p1 - nLineSize : p1;
// 			BYTE* p2 = y < nHeight - 1 ? p1 + nLineSize : p1;
// 			BYTE* p3 = (BYTE*)pEdge + y * nWidth;
// 			for (int x = 0; x < nWidth; x++)
// 			{
// 				BYTE* p4 = x > 0 ? p1 - 1 : p1;
// 				BYTE* p5 = x < nWidth - 1 ? p1 + 1 : p1;
// 				int gx = abs((*p4 + *p5 + *p0 + *p2) - 4 * (*p1));
// 				*p3++ = max(0, min(gx, 255));
// 				p0 += nPixelSize;
// 				p1 += nPixelSize;
// 				p2 += nPixelSize;
// 			}
// 		}

		for (int y = 0; y < nHeight-1; y++)
		{
			BYTE* p1 = (BYTE*)pGray + y * nLineSize;
			BYTE* p2 = p1 + nLineSize;
			BYTE* p3 = (BYTE*)pEdge + y * nWidth;
			for (int x = 0; x < nWidth-1; x++)
			{
				int dx = abs(*p1 - *(p2 + nPixelSize)) + abs(*p2 - *(p1 + nPixelSize));
				*p3++ = max(0, min(dx, 255));
				p1 += nPixelSize;
				p2 += nPixelSize;
			}
		}

	}

	void CImageFilter::CalcMerge(CBitmap* pGraph, CBitmap* pEdge)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nLineBytes = pGraph->LineSize();

		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pG = pGraph->GetBits(0, y);
			BYTE* pE = pEdge->GetBits(0, y);

			for (int x = 0; x < nWidth; x++)
			{
				if (*pE)
				{
					*pG = 128;
				}

				pG++;
				pE++;
			}
		}
	}

	inline void SetPixel(int x, int y, int c, BYTE* p, int w, int h, int bits, int line)
	{
		x = (x<0) ? 0 : ((x>w) ? w : x);
		y = (y<0) ? 0 : ((y>h) ? h : y);
		*(p + y * line + x * (bits >> 3)) = c;
	}

	void CImageFilter::FillRect(int x, int y, int block, int c, BYTE* pData, int nBitCount, int nLineSize)
	{
		for (int y0 = y; y0 < y + block; y0++)
		{
			BYTE* p = pData + y0 * nLineSize + x * (nBitCount >> 3);
			for (int x0 = x; x0 < x + block; x0++)
			{
				*p = c;
				p += nBitCount >> 3;
			}
		}
	}

	// 基于 Bresenham 算法画填充圆
	void CImageFilter::FillRound(int x, int y, int r, int c, BYTE* pData, int nWidth, int nHeight, int nBitCount)
	{
		int tx = 0, ty = r, d = 3 - 2 * r, color = c, w = nWidth - 1, h = nHeight - 1;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		while (tx < ty)
		{
			// 画水平两点连线(<45度)
			for (int i = x - ty; i <= x + ty; i++)
			{
				SetPixel(i, y - tx, color, pData, w, h, nBitCount, nLineSize);
				if (tx != 0)	// 防止水平线重复绘制
				{
					SetPixel(i, y + tx, color, pData, w, h, nBitCount, nLineSize);
				}
			}

			if (d < 0)			// 取上面的点
			{
				d += 4 * tx + 6;
			}
			else				// 取下面的点
			{
				// 画水平两点连线(>45度)
				for (int i = x - tx; i <= x + tx; i++)
				{
					SetPixel(i, y - ty, color, pData, w, h, nBitCount, nLineSize);
					SetPixel(i, y + ty, color, pData, w, h, nBitCount, nLineSize);
				}

				d += 4 * (tx - ty) + 10, ty--;
			}

			tx++;
		}

		if (tx == ty)			// 画水平两点连线(=45度)
		{
			for (int i = x - ty; i <= x + ty; i++)
			{
				SetPixel(i, y - tx, color, pData, w, h, nBitCount, nLineSize);
				SetPixel(i, y + tx, color, pData, w, h, nBitCount, nLineSize);
			}
		}
	}

	void CImageFilter::CalcMerge(CBitmap* pTrimap, CBitmap* pGraph, CBitmap* pEdge)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nBitCount = pTrimap->BitCount();
		int nLineSize = pTrimap->LineSize();
		int nBlockSize = pTrimap->Width() / pGraph->Width();
		BYTE* pData = pTrimap->GetBits();

		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pG = pGraph->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				FillRect(x*nBlockSize, y*nBlockSize, nBlockSize, *pG, pData, nBitCount, nLineSize);
				pG++;
			}
		}

		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pE = pEdge->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				if (*pE == 255)
				{
					int x1 = x * nBlockSize;
					int y1 = y * nBlockSize;
					FillRound(x1, y1, 5, 128, pData, nWidth*nBlockSize, nHeight*nBlockSize, nBitCount);
				}
				pE++;
			}
		}
	}


	void CImageFilter::DarkChannel(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nLineSize = WidthBytes(nWidth*nBitCount);
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* p = (BYTE*)pData + y * nLineSize;
			for (int x = 0; x < nWidth; x++)
			{
				p[0] = p[1] = p[2] = min(p[0], min(p[1], p[2]));
				p += nBitCount >> 3;
			}
		}
	}

	void CImageFilter::RemoveBlock(CBitmap* pGraph, int nMinSize, int nMaxSize, bool bBlackPixel)
	{
		if (m_pRegion == NULL)
		{
			m_pRegion = new CRegion(pGraph->Width(), pGraph->Height());
			assert(m_pRegion);
		}

		m_pRegion->RemoveBlock(pGraph, nMinSize, nMaxSize, bBlackPixel);
	}

	void CImageFilter::RemoveNoise(CBitmap* pGraph)
	{
			Erosion(pGraph, 0x03, true);
			RemoveBlock(pGraph, 1, 20, true);
			Dilation(pGraph, 0x03, true);
			RemoveBlock(pGraph, 1, 20, true);

 //			Erosion(pGraph, 0x03, false);
// 			RemoveBlock(pGraph, 1, 10, false);
//			Dilation(pGraph, 0x03, false);
// 			RemoveBlock(pGraph, 1, 10, false);
	}
}