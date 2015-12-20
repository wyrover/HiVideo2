#include "stdafx.h"
#include "VideoDevice.h"
#include "VideoRender.h"
#include <errors.h>

namespace e
{
	CVideoDevice::CVideoDevice( HRESULT* phr)
	{
		m_eState = Unbuilde;
		m_pGraphBuilder = NULL;
		m_pVideoCapture = NULL;
		m_pVideoDecoder = NULL;
		m_pVideoRender = NULL;
		m_pMediaControl = NULL;
		memset(m_szDeviceName, 0, sizeof(m_szDeviceName));

		SetDefaultConfig();
		*phr = S_OK;
	}

	CVideoDevice::~CVideoDevice(void)
	{
		Destroy();
	}

	HRESULT CVideoDevice::GetCaptureDeviceNames(IVideoDevice* pDeviceCallback)
	{
		CheckPointer(pDeviceCallback, E_POINTER);
		std::vector<TCaptureDevice> devices;
		HRESULT hr = EnumCaptures(CLSID_VideoInputDeviceCategory, devices);
		if (SUCCEEDED(hr))
		{
			for (size_t i = 0; i < devices.size(); i++)
			{
				pDeviceCallback->OnAddDevice(devices[i].szDeviceName);
			}
		}
		return hr;
	}

	HRESULT CVideoDevice::GetCaptureDeviceFormats(LPCTSTR lpDeviceName, IVideoFormat* pFormatCallback)
	{
		CheckPointer(lpDeviceName, E_POINTER);
		CheckPointer(pFormatCallback, E_POINTER);
		//get video capture filter
		IBaseFilter* pVideoCapture = NULL;
		HRESULT hr = GetCaptureFilter(CLSID_VideoInputDeviceCategory, lpDeviceName, (void**)&pVideoCapture);
		if (FAILED(hr)) return hr;
		//get filter config
		IAMStreamConfig* pConfig;
		hr = GetAMCaptureConfig(pVideoCapture, PINDIR_OUTPUT, &pConfig);
		if (FAILED(hr)) return hr;
		//get config format
		std::vector<AM_MEDIA_TYPE> formats;
		hr = GetAMCaptureFormats(pConfig, formats);
		if (SUCCEEDED(hr))
		{
			TCHAR szFormat[MAX_PATH], szSubname[MAX_PATH];
			for (size_t i = 0; i < formats.size(); i++)
			{
				VIDEOINFOHEADER* pFormat = (VIDEOINFOHEADER*)formats[i].pbFormat;
				if (pFormat->bmiHeader.biWidth==0||pFormat->bmiHeader.biHeight==0) 
					continue;
				if (S_OK != GetSubtypeName(formats[i].subtype, szSubname)) 
					continue;

				_stprintf_s(szFormat, _T("%s %dx%d"), szSubname, pFormat->bmiHeader.biWidth, pFormat->bmiHeader.biHeight);
				pFormatCallback->OnAddFormat(szFormat);
			}
		}

		pVideoCapture->Release();
		return hr;
	}

	HRESULT CVideoDevice::GetCurrentDeviceFormats(IVideoFormat* pFormatCallback)
	{
		CheckPointer(pFormatCallback, E_POINTER);
		CheckPointer(m_pVideoCapture, E_INVALIDARG);

		IAMStreamConfig* pConfig;
		HRESULT hr = GetAMCaptureConfig(m_pVideoCapture, PINDIR_OUTPUT, &pConfig);
		if (FAILED(hr)) return hr;
		std::vector<AM_MEDIA_TYPE> formats;
		hr = GetAMCaptureFormats(pConfig, formats);
		if (SUCCEEDED(hr))
		{
			TCHAR szFormat[MAX_PATH], szSubname[MAX_PATH];
			for (size_t i = 0; i < formats.size(); i++)
			{
				VIDEOINFOHEADER* pCfg = (VIDEOINFOHEADER*)formats[i].pbFormat;
				GetSubtypeName(formats[i].subtype, szSubname);
				_stprintf_s(szFormat, _T("%s %dx%d"), szSubname, pCfg->bmiHeader.biWidth, pCfg->bmiHeader.biHeight);
				pFormatCallback->OnAddFormat(szFormat);
			}
		}
		return hr;
	}

	HRESULT CVideoDevice::SetCurrentOutputFormat(int nWidth, int nHeight, VideoType eType)
	{
		HRESULT hr = S_OK;
		if (m_pVideoCapture)
		{
			IAMStreamConfig* pConfig;
			hr = GetAMCaptureConfig(m_pVideoCapture, PINDIR_OUTPUT, &pConfig);
			if (FAILED(hr)) return hr;
			AM_MEDIA_TYPE* pmt;
			hr = pConfig->GetFormat(&pmt);
			if (SUCCEEDED(hr))
			{
				hr = GetSubtype(eType, pmt->subtype);
				if (SUCCEEDED(hr))
				{
					((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biWidth = nWidth;
					((VIDEOINFOHEADER*)pmt->pbFormat)->bmiHeader.biHeight = nHeight;
					hr = pConfig->SetFormat(pmt);
					if (SUCCEEDED(hr))
					{
						m_nVideoWidth = nWidth;
						m_nVideoHeight = nHeight;
						m_eVideoType = eType;
					}
				}
			}
		}
		else
		{
			m_nVideoWidth = nWidth;
			m_nVideoHeight = nHeight;
			m_eVideoType = eType;
		}
		return hr;
	}

	HRESULT CVideoDevice::GetCurrentOutputDevice(LPTSTR lpDeviceName)
	{
		_tcscpy_s(lpDeviceName, MAX_PATH, m_szDeviceName);
		return NOERROR;
	}

	HRESULT CVideoDevice::GetCurrentOutputFormat(LPTSTR lpDeviceFormat)
	{
		return GetVideoTypeName(m_eVideoType, lpDeviceFormat);
	}

	HRESULT CVideoDevice::CreateCapture(LPCTSTR lpDeviceName, IBaseFilter** ppCapture)
	{	
		HRESULT hr = GetCaptureFilter(CLSID_VideoInputDeviceCategory, lpDeviceName, (void**)ppCapture);
		if (FAILED(hr)) return hr;
		hr = SetCaptureConfig(*ppCapture, m_nVideoWidth, m_nVideoHeight);
		if (FAILED(hr)) return hr;
		_tcscpy_s(m_szDeviceName, lpDeviceName);
		return hr;
	}

	HRESULT  CVideoDevice::CreateDecoder(VideoType eType, IBaseFilter** ppDecoder)
	{
		HRESULT hr = S_OK;
		if (eType == MJPG)
		{
			hr = CoCreateInstance(CLSID_MjpegDec, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pVideoDecoder);
			if (FAILED(hr)) return hr;
		}
		else
		{
			*ppDecoder = NULL;
			hr = S_OK;
		}
		return hr;
	}

	HRESULT CVideoDevice::CreateRender(IVideoSample* pSampleCallback, IBaseFilter** ppRender)
	{
		CheckPointer(ppRender, E_POINTER);
		HRESULT hr = S_OK;
		*ppRender = new CVideoRenderer(_T("Video Render Filter"), NULL, &hr);
		if (*ppRender == NULL) return E_OUTOFMEMORY;
		if (FAILED(hr))
		{
			delete *ppRender;
			*ppRender = NULL;
		}
		dynamic_cast<CVideoRenderer*>(*ppRender)->SetSampleCallback(pSampleCallback);
		return S_OK;
	}

	HRESULT CVideoDevice::GetSubtype(VideoType eType, GUID& subtype)
	{
		HRESULT hr = S_OK;
		switch (eType)
		{
		case I420:subtype = MEDIASUBTYPE_I420; break;
		case RGB24:subtype = MEDIASUBTYPE_RGB24; break;
		case RGB32:subtype = MEDIASUBTYPE_RGB32; break;
		case IMC1:subtype = MEDIASUBTYPE_IMC1; break;
		case IMC2:subtype = MEDIASUBTYPE_IMC2; break;
		case IMC3:subtype = MEDIASUBTYPE_IMC3; break;
		case IMC4:subtype = MEDIASUBTYPE_IMC4; break;
		case IYUV:subtype = MEDIASUBTYPE_IYUV; break;
		case YV12:subtype = MEDIASUBTYPE_YV12; break;
		case YUY2:subtype = MEDIASUBTYPE_YUY2; break;
		case YUYV:subtype = MEDIASUBTYPE_YUYV; break;
		case UYVY:subtype = MEDIASUBTYPE_UYVY; break;
		case MJPG:subtype = MEDIASUBTYPE_MJPG; break;
		default: hr = E_FAIL; break;
		}
		return hr;
	}

	HRESULT CVideoDevice::GetSubtypeName(REFGUID subtype, LPTSTR lpTypeName)
	{
		HRESULT hr = S_OK;
		if (subtype == MEDIASUBTYPE_IYUV){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("IYUV"));
		} else if (subtype == MEDIASUBTYPE_IMC1){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("IMC1"));
		}else if (subtype == MEDIASUBTYPE_IMC2){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("IMC2"));
		}else if (subtype == MEDIASUBTYPE_IMC3){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("IMC3"));
		}else if (subtype == MEDIASUBTYPE_IMC4){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("IMC4"));
		}else if (subtype == MEDIASUBTYPE_YV12){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("YV12"));
		}else if (subtype == MEDIASUBTYPE_RGB24){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("RGB24"));
		}else if (subtype == MEDIASUBTYPE_RGB32){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("RGB32"));
		}else if (subtype == MEDIASUBTYPE_YUYV){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("YUYV"));
		}else if (subtype == MEDIASUBTYPE_YUY2){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("YUY2"));
		}else if (subtype == MEDIASUBTYPE_I420){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("I420"));
		}else if (subtype == MEDIASUBTYPE_MJPG){
			_tcscpy_s(lpTypeName, MAX_PATH, _T("MJPG"));
		}else {
			_tcscpy_s(lpTypeName, MAX_PATH, _T("UKNOWN"));
			hr = S_FALSE;
		}
		return hr;
	}

	HRESULT CVideoDevice::GetVideoTypeName(VideoType eType, LPTSTR lpTypeName)
	{
		GUID subtype;
		HRESULT hr = GetSubtype(eType, subtype);
		if (FAILED(hr)) return hr;
		return GetSubtypeName(subtype, lpTypeName);
	}

	HRESULT CVideoDevice::SetDefaultConfig(void)
	{
		m_nVideoWidth = 640;
		m_nVideoHeight = 480;
		m_eVideoType = YUY2;
		return NOERROR;
	}

	HRESULT CVideoDevice::SetCaptureConfig(IBaseFilter* pCapture, int nWidth, int nHeight)
	{
		CheckPointer(pCapture, E_POINTER);
		IAMStreamConfig* pConfig;
		HRESULT hr = GetAMCaptureConfig(pCapture, PINDIR_OUTPUT, &pConfig);
		if (FAILED(hr)) return hr;

		AM_MEDIA_TYPE* pmt;
		hr = pConfig->GetFormat(&pmt);
		if (SUCCEEDED(hr))
		{
			GUID subtype;
			hr = GetSubtype(m_eVideoType, subtype);
			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pFormat = (VIDEOINFOHEADER*)pmt->pbFormat;
				if (IsEqualGUID(subtype,pmt->subtype) && pFormat->bmiHeader.biWidth == nWidth && pFormat->bmiHeader.biHeight == nHeight)
				{
					goto _out;
				}
			}

			hr = GetSubtype(m_eVideoType, pmt->subtype);
			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pFormat = (VIDEOINFOHEADER*)pmt->pbFormat;
				pFormat->bmiHeader.biWidth = nWidth;
				pFormat->bmiHeader.biHeight = nHeight;
				hr = pConfig->SetFormat(pmt);
				if (FAILED(hr))
				{
					ASSERT(0);
				}
			}
		}
_out:
		pConfig->Release();
		return hr;
	}

	HRESULT CVideoDevice::Create(LPCTSTR lpDeviceName,IVideoSample* pSampleCallback)
	{
		CheckPointer(lpDeviceName, E_INVALIDARG);
		if (m_eState != Unbuilde)
		{
			ASSERT(SUCCEEDED(Destroy()));
		}

		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGraphBuilder);
		if (FAILED(hr)) return hr;

		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void**)&m_pCaptureGraphBuilder2);
		if (FAILED(hr)) return hr;

		hr = m_pCaptureGraphBuilder2->SetFiltergraph(m_pGraphBuilder);
		if (FAILED(hr)) return hr;

		hr = CreateCapture(lpDeviceName, &m_pVideoCapture);
		if (FAILED(hr)) return hr;

		hr = CreateDecoder(m_eVideoType, &m_pVideoDecoder);
		if (FAILED(hr)) return hr;
		
		hr = CreateRender(pSampleCallback, &m_pVideoRender);
		if (FAILED(hr)) return hr;

		hr = m_pGraphBuilder->AddFilter(m_pVideoCapture, NAME("VideoCapture"));
		if (FAILED(hr)) return hr;
		m_pVideoCapture->AddRef();

		hr = m_pGraphBuilder->AddFilter(m_pVideoRender, NAME("VideoRender"));
		if (FAILED(hr)) return hr;
		m_pVideoRender->AddRef();

		if (m_eVideoType == MJPG)
		{
			hr = CreateDecoder(m_eVideoType, &m_pVideoDecoder);
			if (FAILED(hr)) return hr;
			hr = m_pGraphBuilder->AddFilter(m_pVideoDecoder, NAME("VideoDecoer"));
			if (FAILED(hr)) return hr;
			m_pVideoDecoder->AddRef();
		}

		hr = m_pCaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE
			, &MEDIATYPE_Video
			, m_pVideoCapture
			, m_pVideoDecoder
			, m_pVideoRender);
		if (FAILED(hr)) return hr;

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
		if (FAILED(hr)) return hr;
		
		m_eState = Builded;
		return Pause();
	}

	HRESULT CVideoDevice::Start(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		if (m_eState == Running) return S_FALSE;
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (SUCCEEDED(hr))
		{
			if (fs != State_Running)
			{
				hr = m_pMediaControl->Run();
				if (SUCCEEDED(hr)) m_eState = Running;
			}
		}
		return hr;
	}

	HRESULT CVideoDevice::Pause(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		if (m_eState == Paused) return S_FALSE;
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (SUCCEEDED(hr))
		{
			if (fs != State_Paused)
			{
				hr = m_pMediaControl->Pause();
				if (SUCCEEDED(hr)) m_eState = Paused;
			}
		}
		return hr;
	}

	HRESULT CVideoDevice::Stop(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		if (m_eState == Stopped) return S_FALSE;
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (SUCCEEDED(hr))
		{
			if (fs != State_Stopped)
			{
				hr = m_pMediaControl->Stop();
				if (SUCCEEDED(hr)) m_eState = Stopped;
			}
		}
		return hr;
	}

	HRESULT CVideoDevice::Destroy(void)
	{
		Stop();

		if (m_pVideoCapture)
		{
			RemoveFilter(m_pGraphBuilder, m_pVideoCapture);
		}
		if (m_pVideoDecoder)
		{
			RemoveFilter(m_pGraphBuilder, m_pVideoDecoder);
		}
		if (m_pVideoRender)
		{
			RemoveFilter(m_pGraphBuilder, m_pVideoRender);
		}

		SafeRelease(&m_pMediaControl);
		SafeRelease(&m_pVideoCapture);
		SafeRelease(&m_pVideoDecoder);
		SafeRelease(&m_pVideoRender);
		SafeRelease(&m_pCaptureGraphBuilder2);
		SafeRelease(&m_pGraphBuilder);
		m_eState = Unbuilde;
		return S_OK;
	}

}
