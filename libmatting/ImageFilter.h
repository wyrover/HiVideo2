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
	protected:
		void CalcKernals(float m_fSigma);
		void Convolve(void* pSrc, void* pDst, int nWidth, int nHeight, int nBitCount);
		void GetBlock(void* pData, int nBitCount, int nLineBytes, int nBlockSize);
		void SetBlock(int nValue, void* pData, int nBitCount, int nLineBytes, int nBlockSize);
		void Erosion(CBitmap* pGraph, int nDirection);//��ʴ�ڵ�
		void Dilation(CBitmap* pGraph, int nDirection);//���ͺڵ�
		void RemoveBlock(CBitmap* pGraph, int nThreshold);
	protected:
		int m_nRadius;
		float m_fSigma;
		float* m_pKernals;
		//blur
		void* m_pTemp;
		CImageBlur* m_pBlur;
		//graph
		CBitmap* m_pGraphTemp;
		CRegion* m_pRegion;
	};
}
