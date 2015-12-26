#pragma once

namespace e
{
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
	protected:
		void CalcKernals(float m_fSigma);
		void Convolve(void* pSrc, void* pDst, int nWidth, int nHeight, int nBitCount);
		void CalcBlock(void* pData, int nBitCount, int nLineBytes, int nBlockSize);
	protected:
		int m_nRadius;
		float m_fSigma;
		float* m_pKernals;

		void* m_pTemp;
	};
}
