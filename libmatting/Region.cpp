#include "stdafx.h"
#include "Region.h"

namespace e
{
	CRegion::CRegion(int nWidth, int nHeight)
	{
		m_nCurrent = 0;
		m_pQueue = NULL;
		m_pTemp = NULL;

		Initialize(nWidth, nHeight);
	}

	CRegion::~CRegion(void)
	{
		Clean();
	}

	bool CRegion::Initialize(int nWidth, int nHeight)
	{
		//init queue size
		if (m_pQueue == NULL)
		{
			m_pQueue = new Point[nWidth * nHeight];
			if (!m_pQueue) return false;
		}
		//create a temp bitmap
		if (m_pTemp == NULL)
		{
			m_pTemp = new CBitmap();
			if (!m_pTemp) return false;
		}
		return true;
	}

	bool CRegion::FindRegion(CBitmap* pBitmap, int nMinCount, bool bBlackPixel)
	{
		m_pTemp->Store(pBitmap);
		int nWidth = pBitmap->Width();
		int nHeight = pBitmap->Height();
		int nBitCount = pBitmap->BitCount();
		int x0, y0, x1, y1, nHead, nRear;
		int nDirect[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		int nValue[2] = { 0 };

		if (bBlackPixel) {
			nValue[0] = 0; nValue[1] = 255;
		}else{
			nValue[0] = 255; nValue[1] = 0;
		}

		nHead = nRear = 0;
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
				uint8* p0 = m_pTemp->GetBits(x, y);
				if (*p0 != nValue[0]) continue;
				*p0 = nValue[1];

				Point point(x, y);
				m_pQueue[nRear++] = point;
				m_Regions[m_nCurrent].points.push_back(point);

				while (nHead < nRear)
				{
					point = m_pQueue[nHead++];

					for (int i = 0; i < 4; i++)
					{
						x1 = point.x + nDirect[i][0];
						y1 = point.y + nDirect[i][1];
						if ((x1 >= 0 && x1 < nWidth) && (y1 >= 0 && y1 < nHeight))
						{
							uint8* p1 = m_pTemp->GetBits(x1, y1);
							if (*p1 == nValue[0])
							{
								*p1 = nValue[1];
								point.x = x1; point.y = y1;
								m_pQueue[nRear++] = point;
								m_Regions[m_nCurrent].points.push_back(point);
							}
						}
					}
				}//end while (nHead < nRear)

				if (m_Regions[m_nCurrent].points.size() < (size_t)nMinCount)
				{
					m_nCurrent++;
				}
				else
				{
					m_Regions[m_nCurrent].points.clear();
				}
			}
		}

		return true;
	}

	void CRegion::EraseRegion(CBitmap* pBitmap, Item& item, int nValue)
	{
		for (size_t i = 0; i < item.points.size(); i++)
		{
			int x = item.points[i].x, y = item.points[i].y;
			uint8* p = pBitmap->GetBits(x, y);
			*p = nValue;
		}
	}

	bool CRegion::RemoveBlock(CBitmap* pBitmap, int nMinCount, bool bBlackPixel)
	{
		m_nCurrent = 0;
		FindRegion(pBitmap, nMinCount, bBlackPixel);

		for (int i = 0; i < m_nCurrent; i++)
		{
			EraseRegion(pBitmap, m_Regions[i], bBlackPixel ? 255 : 0);
		}

		return true;
	}

	void CRegion::Clean(void)
	{
		SafeDelete(&m_pQueue);
		SafeDelete(&m_pTemp);
	}
}
