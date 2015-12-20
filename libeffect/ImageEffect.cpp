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

		uint8* pDst = (uint8*)pData;
		uint8* pSrc = (uint8*)m_pBlockTemp->GetData();
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
// 				pDst[0] = abs(pSrc[0] - pDst[0]) + pDst[0];
// 				pDst[1] = abs(pSrc[1] - pDst[1]) + pDst[1];
// 				pDst[2] = abs(pSrc[2] - pDst[2]) + pDst[2];
// 				pDst[3] = abs(pSrc[3] - pDst[3]) + pDst[3];

				pDst[0] = abs(pSrc[0] - pDst[0]) + 128;
				pDst[1] = abs(pSrc[1] - pDst[1]) + 128;
				pDst[2] = abs(pSrc[2] - pDst[2]) + 128;
				pDst[3] = abs(pSrc[3] - pDst[3]) + 128;

				pSrc += 4;
				pDst += 4;
			}
		}
	}
}
