#include "stdafx.h"
#include "ImageEffect.h"
#include "ImageBlur.h"

namespace e
{
	CImageEffect::CImageEffect()
	{
		m_pConfig = 0;
		m_pImageBlur = new CImageBlur();
		assert(m_pImageBlur);
		m_pBlockTemp = new CMemBlock();
		assert(m_pBlockTemp);
	}

	CImageEffect::~CImageEffect()
	{
		Clean();
	}

	void CImageEffect::SetType(int nType)
	{
		m_pImageBlur->SetType(nType);
	}

	void CImageEffect::SetConfig(IEffectConfig* pConfig)
	{

	}

	bool CImageEffect::Init(int nSize)
	{
		return m_pBlockTemp->Create(nSize);
	}

	void CImageEffect::Clean(void)
	{
		SafeDelete(&m_pBlockTemp);
		SafeDelete(&m_pImageBlur);
	}

	inline uint8 mix(uint8 & a, uint8 & b, float & f)
	{
		return a * (1.0f - f) + b * f;
	}

	void CImageEffect::OnSampleProc(void* pData
		, int nSize
		, int nWidth
		, int nHeight
		, int nBitCount)
	{
		bool bRet = Init(nSize);
		assert(bRet);

		m_pBlockTemp->SetData(pData, nSize);

		m_pImageBlur->OnSampleProc(pData
			, nSize
			, nWidth
			, nHeight
			, nBitCount);
		return;

		float fAlpha = 0.7f;
		uint8* pDst = (uint8*)pData;
		uint8* pSrc = (uint8*)m_pBlockTemp->GetData();
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
				pDst[0] = mix(pSrc[0], pDst[0], fAlpha);
				pDst[1] = mix(pSrc[1], pDst[1], fAlpha);
				pDst[2] = mix(pSrc[2], pDst[2], fAlpha);
				pDst[3] = mix(pSrc[3], pDst[3], fAlpha);

				pSrc += 4;
				pDst += 4;
			}
		}
	}
}
