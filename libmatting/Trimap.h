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
		void Erosion(CBitmap* pGraph, int nDirection);//��ʴ�ڵ�
		void Dilation(CBitmap* pGraph, int nDirection);//���ͺڵ�
		void RemoveNoise(void);
		void EdgeDetect(void);
		void SetBlock(BYTE* pData, int nBitCount, int nLineBytes, int nBlockSize, int nValue);
		void CalcTrimap(CBitmap* pTrimap, int nBlockSize);
		void CalcAlpha(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, CBitmap* pFG);
		void CalcAlpha(CBitmap* pAlpha, CBitmap* pBG, CBitmap* FG, int x, int y);
		int GetPointValue(int x, int y, CBitmap* pBitmap, int nWidth, int nHeight);
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

