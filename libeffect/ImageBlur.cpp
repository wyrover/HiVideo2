#include "stdafx.h"
#include "ImageBlur.h"
#include "MemBlock.h"
#include "FastBlur.h"
#include <tchar.h>

namespace e
{
	CImageBlur::CImageBlur(void)
	{
		m_nType = 0;
		m_fSigma = 2.5f;
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

	void CImageBlur::SetType(int nType)
	{
		m_nType = nType;
		TCHAR szMsg[256] = { 0 };
		_stprintf_s(szMsg, 256, _T("blur type = %d\n"), nType);
		OutputDebugString(szMsg);
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

		if (m_nType == 0)
		{
			FastBlur(pTemp, pData, nWidth, nHeight, nBitCount, m_fSigma, Normal);
			FastBlur(pData, pTemp, nHeight, nWidth, nBitCount, m_fSigma, Normal);
		}
		else
		{
			FastBlur(pTemp, pData, nWidth, nHeight, nBitCount, m_fSigma, NormalSSE);
			FastBlur(pData, pTemp, nHeight, nWidth, nBitCount, m_fSigma, NormalSSE);
		}
	}
}