#pragma once

namespace e
{
	class CRegion;
	class CImageBlur;
	class CImageFilter
	{
	public:
		CImageFilter(void);
		virtual ~CImageFilter(void);
		bool Initialize(int nWidth, int nHeight, int nBitCount);
		void Smooth(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SplitBlock(void* pData, int nSize, int nWidth, int nHeight, int nBitCount, int nBlockSize);
		void ConvertGray(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void ConvertGray(void* pGray, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void ConvertGray(CBitmap* pImage);
		void Block2Graph(CBitmap* pGraph, CBitmap* pBlock, int nBlockSize);
		void Graph2Block(CBitmap* pBlock, CBitmap* pGraph, int nBlockSize);
		void RemoveNoise(CBitmap* pGraph);
		void CalcEdge(CBitmap* pGraph, CBitmap* pEdge);
		void CalcEdge(void* pGray, int nSize, int nWidth, int nHeight, int nBitCount);
		void CalcEdge(void* pEdge, void* pGray, int nSize, int nWith, int nHeight, int nBitCount);
		void CalcMerge(CBitmap* pGraph, CBitmap* pEdge);
		void CalcMerge(CBitmap* pTrimap, CBitmap* pGraph, CBitmap* pEdge);
		void CalcAlpha(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, CBitmap* pFG);
		void FillRect(int x, int y, int size, int c, BYTE* pData, int nBitCount, int nLineSize);
		void FillRound(int x, int y, int r, int c, BYTE* pData, int nWidth, int nHeight, int nBitCount);
		void DarkChannel(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		void CalcKernals(float m_fSigma);
		void Convolve(void* pSrc, void* pDst, int nWidth, int nHeight, int nBitCount);
		void GetBlock(void* pData, int nBitCount, int nLineBytes, int nBlockSize);
		void SetBlock(int nValue, void* pData, int nBitCount, int nLineBytes, int nBlockSize);
		void Erosion(CBitmap* pGraph, int nDirection, bool bBlackPixel);//∏Ø ¥
		void Dilation(CBitmap* pGraph, int nDirection, bool bBlackPixel);//≈Ú’Õ
		void RemoveBlock(CBitmap* pGraph, int nMinSize, int nMaxSize, bool bBlackPixel);

		void CalcAlpha(CBitmap* pAlpha, CBitmap* pBG, CBitmap* FG, int x, int y);
		int GetPointValue(int x, int y, CBitmap* pBitmap, int nWidth, int nHeight);
	protected:
		int m_nRadius;
		float m_fSigma;
		float* m_pKernals;
		//smooth
		void* m_pTemp;
		CImageBlur* m_pBlur;
		//graph
		CBitmap* m_pGraphTemp;
		CRegion* m_pRegion;
	};
}
