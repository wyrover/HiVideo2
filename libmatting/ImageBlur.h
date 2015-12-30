#pragma once
namespace e
{
	class CMemBlock;
	class CImageBlur 
	{
	public:
		CImageBlur(void);
		virtual ~CImageBlur(void);
		virtual void SetType(int nType);
		virtual void OnSampleProc(void* pData
			, int nSize
			, int nWidth
			, int nHeight
			, int nBitCount);
	protected:
		bool Initialize(int nSize);
		void Clean(void);
	protected:
		int m_nType;
		float m_fSigma;
		CMemBlock* m_pBlockTemp;
	};
}
