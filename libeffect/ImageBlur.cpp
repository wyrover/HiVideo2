#include "stdafx.h"
#include "ImageBlur.h"
#include "FastBlur.h"


namespace e
{
	CImageBlur::CImageBlur(void)
	{
		m_fSigma = 2.5f;
		m_pBuffer = NULL;
		m_nBufferSize = 0;
	}

	CImageBlur::~CImageBlur(void)
	{
		Clean();
	}

	bool CImageBlur::Init(int nSize)
	{
		if (nSize <= 0) return false;
		if (nSize > m_nBufferSize)
		{
			m_pBuffer = realloc(m_pBuffer, nSize);
			if (m_pBuffer == NULL) return false;
			m_nBufferSize = nSize;
		}
		return true;
	}

	void CImageBlur::Clean(void)
	{
		if (m_pBuffer)
		{
			free(m_pBuffer);
			m_pBuffer = NULL;
		}
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

		FastBlur(m_pBuffer, pData, nWidth, nHeight, nBitCount, m_fSigma, NormalSSE);
		FastBlur(pData, m_pBuffer, nHeight, nWidth, nBitCount, m_fSigma, NormalSSE);
	}
}