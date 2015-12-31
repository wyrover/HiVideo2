#include "stdafx.h"
#include "VideoRender.h"

namespace e
{
#ifndef  WidthBytes
#define WidthBytes(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#endif // !WIDTHBYTES

	const int kOutVideoBitCount = 32;

	void SwapLine(BYTE* src, BYTE* dst, int nSize)
	{
		int* a = (int*)src;
		int* b = (int*)dst;
		for (int i = 0; i < nSize / 4; i++)
		{
			int t = *a;
			*a = *b;
			*b = t;

			a++;
			b++;
		}
	}

	CVideoRenderer::CVideoRenderer(LPCTSTR lpName, LPUNKNOWN lpUnk, HRESULT* phr)
		: CBaseRenderer(CLSID_VideoOutputFilter, lpName, lpUnk, phr)
	{
		m_pSampleCallback = NULL;
		m_pOutBuffer = NULL;
		m_nOutWidth = kOutVideoWidth;
		m_nOutHeight = kOutVideoHeight;
		m_nOutBitCount = kOutVideoBitCount;
		m_ffScaler.SetAttribute(SWS_FMT_RGB24, SWS_FMT_BGRA, SWS_SA_BILINEAR);

		SetOutputFormat(kOutVideoWidth, kOutVideoHeight);
	}

	CVideoRenderer::~CVideoRenderer(void)
	{
		if (m_pOutBuffer)
		{
			free(m_pOutBuffer);
			m_pOutBuffer = NULL;
		}
	}

	void CVideoRenderer::SetSampleCallback(IVideoSample* pSampleCallback)
	{
		CAutoLock lock(&m_cStateLock);
		m_pSampleCallback = pSampleCallback;
	}

	void CVideoRenderer::SetOutputFormat(int nWidth, int nHeight)
	{
		CAutoLock lock(&m_cStateLock);
		HRESULT hr = CreateRGBABuffer(nWidth, nHeight, kOutVideoBitCount);
		if (SUCCEEDED(hr))
		{
			m_nOutWidth = nWidth;
			m_nOutHeight = nHeight;
			m_nOutBitCount = kOutVideoBitCount;
		}
		else
		{
			ASSERT(0);
		}
	}

	HRESULT CVideoRenderer::GetVideoType(REFCLSID subtype, VideoType& eType)
	{
		if (IsEqualGUID(subtype, MEDIASUBTYPE_IYUV)){
			eType = IYUV;
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_RGB24)){
			eType = RGB24;
			m_ffScaler.SetAttribute(SWS_FMT_BGR24, SWS_FMT_BGRA, SWS_SA_BILINEAR);
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_RGB32)){
			eType = RGB32;
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_YV12)){
			eType = YV12;
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_UYVY)||IsEqualGUID(subtype, MEDIASUBTYPE_HDYC)){
			eType = UYVY;
			m_ffScaler.SetAttribute(SWS_FMT_UVVY422, SWS_FMT_BGRA, SWS_SA_BILINEAR);
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_YUY2)){
			eType = YUY2;
			m_ffScaler.SetAttribute(SWS_FMT_YUV422, SWS_FMT_BGRA, SWS_SA_BILINEAR);
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_YVYU)){
			eType = YVYU;
		}else if (IsEqualGUID(subtype, MEDIASUBTYPE_I420)){
			eType = I420;
		}else{
			eType = UNKNOWN;
		}
		
		return eType == UNKNOWN ? E_FAIL : S_OK;
	}

	HRESULT CVideoRenderer::CreateRGBABuffer(int nWidth, int nHeight, int nBitCount)
	{
		ASSERT(nWidth >= 0 && nHeight >= 0 && nBitCount > 0);
		int nSize = WidthBytes(nBitCount * nWidth) * nHeight;
		m_pOutBuffer = (BYTE*)realloc(m_pOutBuffer, nSize);
		if (m_pOutBuffer == NULL) return E_OUTOFMEMORY;
		memset(m_pOutBuffer, 0, nSize);
		return S_OK;
	}

	HRESULT CVideoRenderer::ReverseBitmap(BYTE* pData, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nWidth*nBitCount);
		for (int y = 0; y < nHeight / 2; y++)
		{
			BYTE* p0 = pData + y * nLineBytes;
			BYTE* p1 = pData + (nHeight - 1 - y)*nLineBytes;
			SwapLine(p0, p1, nLineBytes);
		}
		return S_OK;
	}

	HRESULT CVideoRenderer::CheckMediaType(const CMediaType* pmt)
	{
		CheckPointer(pmt, E_POINTER);
		CAutoLock lock(&m_cStateLock);
		if (pmt->majortype != MEDIATYPE_Video) return E_FAIL;
		HRESULT hr = GetVideoType(pmt->subtype, m_eVideoType);
		if (FAILED(hr)) return hr;
		m_nWidth = ((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biWidth;
		m_nHeight = ((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biHeight;
		m_nBitCount = ((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biBitCount;
		m_mt = *pmt;
		return S_OK;
	}

	HRESULT CVideoRenderer::DoRenderSample(IMediaSample *pMediaSample)
	{
		CAutoLock lock(&m_cStateLock);
		if (m_pSampleCallback)
		{
			BYTE* pData = NULL;
			HRESULT hr = pMediaSample->GetPointer(&pData);
			LONG lSize = pMediaSample->GetActualDataLength();
			VIDEOINFOHEADER* pCfg = (VIDEOINFOHEADER*)m_mt.pbFormat;
			int nWidth = pCfg->bmiHeader.biWidth;
			int nHeight = pCfg->bmiHeader.biHeight;
			int nBitCount = pCfg->bmiHeader.biBitCount;

			if (m_eVideoType == YUY2)
			{
				int nLineBytes = WidthBytes(m_nOutWidth* m_nOutBitCount);
				bool bRet = m_ffScaler.Scale(pData, nWidth, nHeight, nWidth * 2, m_pOutBuffer, m_nOutWidth, m_nOutHeight, nLineBytes);
				pData = m_pOutBuffer;
				lSize = nLineBytes *  m_nOutHeight;
				nWidth = m_nOutWidth;
				nHeight = m_nOutHeight;
				nBitCount = m_nOutBitCount;
			}
			else if (m_eVideoType == RGB24)
			{
				//int nInLineBytes = WidthBytes(nBitCount*nWidth);
				int nInLineBytes = nWidth * 3;
				int nOutLineBytes = WidthBytes(m_nOutBitCount*m_nOutWidth);
				bool bRet = m_ffScaler.Scale(pData, nWidth, nHeight, nInLineBytes, m_pOutBuffer, m_nOutWidth, m_nOutHeight, nOutLineBytes);
				ReverseBitmap(m_pOutBuffer, m_nOutWidth, m_nOutHeight, m_nOutBitCount);
				pData = m_pOutBuffer;
				lSize = nOutLineBytes *  m_nOutHeight;
				nWidth = m_nOutWidth;
				nHeight = m_nOutHeight;
				nBitCount = m_nOutBitCount;
			}
			//samples callback
			m_pSampleCallback->OnAddSample(pData, lSize, nWidth, nHeight, nBitCount);
#if 0
			TCHAR szMsg[1024] = { 0 };
			_stprintf_s(szMsg, _T("video sample render:%d,%d,%d,%d\n"), lSize, nWidth, nHeight, nBitCount);
			OutputDebugString(szMsg);
#endif
		}
		return S_OK;
	}
}
