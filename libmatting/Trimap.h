#pragma once

namespace e
{
	class CRegion;
	class CTrimap
	{
	public:
		CTrimap(void);
		CTrimap(int nWidth, int nHeight, int nBlockSize);
		virtual ~CTrimap(void);
		bool Initialize(int nWidth, int nHeight, int nBlockSize);
		void Calculate(CBitmap* pTrimap, CBitmap* pBitmap, int nBlockSize);
	protected:
		void CalcBlock(BYTE* pData, int nBitCount, int nLineBytes, int nBlockSize);
		void BlockingAndGraph(CBitmap* pBitmap, int nBlockSize);
		void Erosion(CBitmap* pGraph, int nDirection);//¸¯Ê´ºÚµã
		void Dilation(CBitmap* pGraph, int nDirection);//ÅòÕÍºÚµã
		void RemoveNoise(void);
		void EdgeDetect(void);
		void SetBlock(BYTE* pData, int nBitCount, int nLineBytes, int nBlockSize, int nValue);
		void CreateTrimap(CBitmap* pTrimap, int nBlockSize);
	protected:
		int m_nWidth;
		int m_nHeight;
		int m_nBlockSize;
		CBitmap* m_pGraph;
		CBitmap* m_pEdge;
		CBitmap* m_pTemp;
		CRegion* m_pRegion;
	};
}

