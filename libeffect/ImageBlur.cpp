#include "stdafx.h"
#include "ImageBlur.h"
#include "MemBlock.h"
#include "FastBlur.h"

namespace e
{
	CImageBlur::CImageBlur(void)
	{
		m_fSigma = 3.5f;
		m_pBlockTemp = new CMemBlock();
		assert(m_pBlockTemp);
	}

	CImageBlur::~CImageBlur(void)
	{
		Clean();
	}

	bool CImageBlur::Init(int nSize)
	{
		return m_pBlockTemp->Create(nSize);
	}

	void CImageBlur::Clean(void)
	{
		SafeDelete(&m_pBlockTemp);
	}

	void CImageBlur::SetConfig(IEffectConfig* pConfig)
	{

	}

	void CImageBlur::OnSampleProc(void* pData
		, int nSize
		, int nWidth
		, int nHeight
		, int nBitCount)
	{
		Init(nSize);

		void* pTemp = m_pBlockTemp->GetData();
		FastBlur(pTemp, pData, nWidth, nHeight, nBitCount, m_fSigma, NormalSSE);
		FastBlur(pData, pTemp, nHeight, nWidth, nBitCount, m_fSigma, NormalSSE);
	}
}