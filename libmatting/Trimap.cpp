#include "stdafx.h"
#include "Trimap.h"
#include "Region.h"

namespace e
{
	CTrimap::CTrimap(void)
	{
		m_nWidth = 0;
		m_nHeight = 0;
		m_nBlockSize = 0;
		m_pEdge = new CBitmap();
		m_pGraph = new CBitmap();
		m_pTemp = new CBitmap();
		m_pRegion = new CRegion();
	}

	CTrimap::CTrimap(int nWidth, int nHeight, int nBlockSize)
	{
		m_nWidth = 0;
		m_nHeight = 0;
		m_nBlockSize = 0;
		m_pEdge = new CBitmap();
		m_pGraph = new CBitmap();
		m_pTemp = new CBitmap();
		m_pRegion = new CRegion();
		//init trimap
		Initialize(nWidth, nHeight, nBlockSize);
	}

	CTrimap::~CTrimap(void)
	{
		SafeDelete(&m_pEdge);
		SafeDelete(&m_pGraph);
		SafeDelete(&m_pTemp);
		SafeDelete(&m_pRegion);
	}

	bool CTrimap::Initialize(int nWidth, int nHeight, int nBlockSize)
	{
		nWidth /= nBlockSize;
		nHeight /= nBlockSize;
		if (!m_pGraph->Create(nWidth, nHeight , 8, NULL, true))
			return false;
		if (!m_pEdge->Create(nWidth, nHeight, 8, NULL, true))
			return false;
		if (!m_pTemp->Create(nWidth, nHeight, 8, NULL, true))
			return false;
		if (!m_pRegion->Initialize(nWidth, nHeight))
			return false;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nBlockSize = nBlockSize;
		return true;
	}

	void CTrimap::CalcBlock(BYTE* pData, int nBitCount, int nLineBytes, int nBlockSize)
	{
		int nSum[3] = { 0 }, nSize = nBlockSize*nBlockSize;
		for (int y = 0; y < nBlockSize; y++)
		{
			BYTE* pDst = pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				nSum[0] += pDst[0];
				nSum[1] += pDst[1];
				nSum[2] += pDst[2];
				pDst += nBitCount >> 3;
			}
		}

		nSum[0] /= nSize;
		nSum[1] /= nSize;
		nSum[2] /= nSize;
		for (int y = 0; y < nBlockSize; y++)
		{
			BYTE* pDst = pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				pDst[0] = nSum[0];
				pDst[1] = nSum[1];
				pDst[2] = nSum[2];
				pDst += nBitCount >> 3;
			}
		}
	}

	void CTrimap::BlockingAndGraph(CBitmap* pBitmap, int nBlockSize)
	{
		int nWidth = pBitmap->Width();
		int nHeight = pBitmap->Height();
		int nBitCount = pBitmap->BitCount();
		int nLineSize = pBitmap->LineSize();

		m_pGraph->Clear();
		for (int y = 0; y < nHeight; y += nBlockSize)
		{
			BYTE* pBlock = pBitmap->GetBits(0, y);
			BYTE* pGraph = m_pGraph->GetBits(0, y / nBlockSize);
			for (int x = 0; x < nWidth; x += nBlockSize)
			{
				CalcBlock(pBlock, nBitCount, nLineSize, nBlockSize);
				*pGraph++ = *pBlock;
				pBlock += nBlockSize*(nBitCount >> 3);
			}
		}
	}

	void CTrimap::Erosion(CBitmap* pGraph, int nDirection)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nBitCount = pGraph->BitCount();
		m_pTemp->Store(pGraph);

		//水平方向
		if (nDirection & 0x01)
		{
			for (int y = 0; y < nHeight; y++)
			{
				uint8* pSrc = m_pTemp->GetBits(0, y);
				uint8* pDst = pGraph->GetBits(0, y);
				for (int x = 0; x < nWidth; x++)
				{
					uint8* p0 = x > 0 ? pSrc - 1 : pSrc;
					uint8* p2 = x < nWidth - 1 ? pSrc + 1 : pSrc;
					//相邻有一个是白点
					if (*p0 || *p2)
						*pDst = 255;
					else
						*pDst = 0;

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
				uint8* pSrc = m_pTemp->GetBits(x, 0);
				uint8* pDst = pGraph->GetBits(x, 0);
				for (int y = 0; y < nHeight; y++)
				{
					uint8* p0 = y > 0 ? pSrc - nLineBytes : pSrc;
					uint8* p2 = y < nHeight - 1 ? pSrc + nLineBytes : pSrc;

					if (*p0 || *p2)
						*pDst = 255;
					else
						*pDst = 0;

					pSrc += nLineBytes;
					pDst += nLineBytes;
				}
			}
		}
	}

	void CTrimap::Dilation(CBitmap* pGraph, int nDirection)
	{
		int nWidth = pGraph->Width();
		int nHeight = pGraph->Height();
		int nBitCount = pGraph->BitCount();
		m_pTemp->Store(pGraph);

		//水平方向
		if (nDirection & 0x01)
		{
			for (int y = 0; y < nHeight; y++)
			{
				uint8* pSrc = m_pTemp->GetBits(0, y);
				uint8* pDst = pGraph->GetBits(0, y);
				for (int x = 0; x < nWidth; x++)
				{
					uint8* p0 = x > 0 ? pSrc - 1 : pSrc;
					uint8* p2 = x < nWidth - 1 ? pSrc + 1 : pSrc;

					if (!*p0 || !*p2)
						*pDst = 0;
					else
						*pDst = 255;

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
				uint8* pSrc = m_pTemp->GetBits(x, 0);
				uint8* pDst = pGraph->GetBits(x, 0);
				for (int y = 0; y < nHeight; y++)
				{
					uint8* p0 = y > 0 ? pSrc - nLineBytes : pSrc;
					uint8* p2 = y < nHeight - 1 ? pSrc + nLineBytes : pSrc;

					if (!*p0 || !*p2)
						*pDst = 0;
					else
						*pDst = 255;

					pSrc += nLineBytes;
					pDst += nLineBytes;
				}
			}
		}
	}

	void CTrimap::RemoveNoise(void)
	{
		int nMinSize = 1, nMaxSize = 50;
		m_pRegion->RemoveBlock(m_pGraph, nMinSize, nMaxSize, true);
	}

	void CTrimap::EdgeDetect(void)
	{
		int nWidth = m_pGraph->Width();
		int nHeight = m_pGraph->Height();
		int nLineSize = m_pGraph->LineSize();
		//laplacian算子
		for (int y = 1; y < nHeight - 1; y++)
		{
			uint8* p1 = m_pGraph->GetBits(1, y);
			uint8* p0 = p1 - nLineSize;
			uint8* p2 = p1 + nLineSize;
			uint8* p3 = m_pEdge->GetBits(1, y);
			for (int x = 1; x < nWidth - 1; x++)
			{
				int gx = abs((*(p1 - 1) + *(p1 + 1) + *p0 + *p2) - 4 * (*p1));
				*p3++ = min(gx, 255);// & *p1
				p0++;
				p1++;
				p2++;
			}
		}
	}

	inline void CTrimap::SetBlock(BYTE* pData, int nBitCount, int nLineBytes, int nBlockSize, int nValue)
	{
		for (int y = 0; y < nBlockSize; y++)
		{
			BYTE* pDst = pData + y * nLineBytes;
			for (int x = 0; x < nBlockSize; x++)
			{
				pDst[0] = nValue;
				pDst[1] = nValue;
				pDst[2] = nValue;
				pDst += nBitCount >> 3;
			}
		}
	}

	void CTrimap::CreateTrimap(CBitmap* pTrimap, int nBlockSize)
	{
		//graph & edge -> trimap
		int nWidth = pTrimap->Width();
		int nHeight = pTrimap->Height();
		int nBitCount = pTrimap->BitCount();
		int nLineSize = pTrimap->LineSize();

		for (int y = 0; y < nHeight; y+=nBlockSize)
		{
			BYTE* pE = m_pEdge->GetBits(0, y / nBlockSize);
			BYTE* pG = m_pGraph->GetBits(0, y / nBlockSize);
			BYTE* pT = pTrimap->GetBits(0, y);
			for (int x = 0; x < nWidth; x+=nBlockSize)
			{
				int nValue = *pE ? 128 : *pG;
				SetBlock(pT, nBitCount, nLineSize, nBlockSize, nValue);
				pG++;
				pE++;
				pT += nBlockSize * (nBitCount >> 3);
			}
		}
	}

	void CTrimap::Calculate(CBitmap* pTrimap, CBitmap* pBitmap, int nBlockSize)
	{
		//1、blocking and create graph
		BlockingAndGraph(pBitmap, nBlockSize);
		//2、remove noise from graph
		RemoveNoise();
		//3、edge detect
		EdgeDetect();
		//4、create trimap
		CreateTrimap(pTrimap, nBlockSize);
	}
}

