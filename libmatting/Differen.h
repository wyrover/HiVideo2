#pragma once
namespace e
{
	class CDifferen
	{
	public:
		CDifferen();
		virtual ~CDifferen();
		void SetThreshold(int nThreshod);
		void Differen(CBitmap* pAlpha, CBitmap* pBG, CBitmap* pFG);
		void Differen(void* pData, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void Sub(void* pData, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void SampleWeight(void* pData, int nWidth, int nHeight, int nBitCount, int nThreshold);
	protected:
		int m_nThreshold;
	};
}