#include "stdafx.h"
#include "ImageFilter.h"
#include "ColorSpace.h"

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
		m_nRadius = 1;
		m_fSigma = 1.0f;
		m_pKernals = 0;
		m_pTemp = NULL;
		CalcKernals(m_fSigma);
	}

	CImageFilter::~CImageFilter(void)
	{
		SafeFree(&m_pTemp);
		SafeFree(&m_pKernals);
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
		if (m_pTemp == NULL)
		{
			Initialize(nHeight, nWidth, nBitCount);
		}

		Convolve(pData, m_pTemp, nWidth, nHeight, nBitCount);
		Convolve(m_pTemp, pData, nHeight, nWidth, nBitCount);
	}

	void CImageFilter::CalcBlock(void* pData, int nBitCount, int nLineBytes, int nBlockSize)
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
				CalcBlock(pSrc, nBitCount, nLineBytes, nBlockSize);
				pSrc += (nBlockSize*(nBitCount >> 3));
			}
		}
	}

	void CImageFilter::ConvertGray(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nBitCount * nWidth);
		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = (uint8*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x++)
			{
				int R = p[0], G = p[1], B = p[2];
				p[3] = p[2] = p[1] = p[0] = (R * 76 + G * 150 + B * 30 + 128) >> 8;
				//p[3] = p[2] = p[1] = p[0] = (R + B + G) / 3;
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
				//RGB2LAB(pSrc[0], pSrc[1], pSrc[2], &lval, &aval, &bval);
				//pDst[x] = max(0, min(lval * 255 / 100, 255));
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
}