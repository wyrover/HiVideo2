#pragma once
#include <vector>

namespace e
{
#define MAX_REGION 256
	class CRegion
	{
	public:
		CRegion(void);
		CRegion(int nWidth, int nHeight);
		virtual ~CRegion(void);
		bool Initialize(int nWidth, int nHeight);
		bool RemoveBlock(CBitmap* pBitmap, int nMinSize, int nMaxSize, bool bBlackPixel);
	protected:	
		struct Point{
			int x, y;
			Point(void)
			{ x = y = 0; }
			Point(int x, int y)
			{ this->x = x; this->y = y; }
			Point(const Point& pt)
			{this->x = pt.x; this->y = pt.y;}
		};

		struct Item{	
			std::vector<Point> points;
		};

		bool FindRegion(CBitmap* pBitmap, int nMinSize, int nMaxSize, bool bBlackPixel);
		void EraseRegion(CBitmap* pBitmap, Item& item, int nValue);

		int m_nCurrent;
		Item m_Regions[MAX_REGION];
		Point* m_pQueue;
		CBitmap* m_pTemp;
	};
}
