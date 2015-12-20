#include "stdafx.h"
#include "ImageEffect.h"
#include "ImageBlur.h"
#include "libutildefs.h"

namespace e
{
	CImageEffect::CImageEffect()
	{
		m_pConfig = 0;
		m_pBuffer = 0;
		m_pImageBlur = new CImageBlur();
		assert(m_pImageBlur);
	}

	CImageEffect::~CImageEffect()
	{
		Clean();
	}

	void CImageEffect::SetConfig(IEffectConfig* pConfig)
	{

	}

	bool CImageEffect::Init(int nSize)
	{
		m_pBuffer = malloc(nSize);
		return m_pBuffer != 0;
	}

	void CImageEffect::Clean(void)
	{
		if (m_pImageBlur)
			delete m_pImageBlur;
		m_pImageBlur = 0;
		if (m_pBuffer) 
			free(m_pBuffer);
		m_pBuffer = 0;
	}

	void CImageEffect::OnSampleProc(void* pData
		, int nSize
		, int nWidth
		, int nHeight
		, int nBitCount)
	{
		if (m_pBuffer == 0)
		{
			Init(nSize);
		}

		memcpy(m_pBuffer, pData, nSize);

		m_pImageBlur->OnSampleProc(pData
			, nSize
			, nWidth
			, nHeight
			, nBitCount);

		int nLineSize = WidthBytes(nWidth * nBitCount);
		uint8* pSrc = (uint8*)m_pBuffer;
		uint8* pDst = (uint8*)pData;
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
				pDst[0] = abs(pSrc[0] - pDst[0]) + pDst[0];
				pDst[1] = abs(pSrc[1] - pDst[1]) + pDst[1];
				pDst[2] = abs(pSrc[2] - pDst[2]) + pDst[2];
				pDst[3] = abs(pSrc[3] - pDst[3]) + pDst[3];

				pSrc += 4;
				pDst += 4;
			}
		}
	}
}
