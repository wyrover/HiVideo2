#include "stdafx.h"
#include "DShowUtils.h"
#include "libutildefs.h"
#include <strmif.h>
#include <uuids.h>

#pragma comment(lib, "strmiids.lib")

namespace e
{
#ifndef CheckPointer
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}
#endif

	HRESULT EnumCaptures(REFCLSID CLSID_DeviceCategory, std::vector<TCaptureDevice>& refDeviceList)
	{
		refDeviceList.clear();

		ICreateDevEnum *pCreateDevEnum = 0;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);
		if (FAILED(hr)) return hr;

		IEnumMoniker* pEnumMoniker = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_DeviceCategory, &pEnumMoniker, 0);
		if (FAILED(hr))
		{
			pCreateDevEnum->Release();
			return hr;
		}
		pEnumMoniker->Reset();

		IMoniker* pMoniker = NULL;
		while (pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK)
		{
			IPropertyBag *pBag = NULL;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{
				VARIANT var = { 0 };
				var.vt = VT_BSTR;
				hr = pBag->Read(_T("FriendlyName"), &var, NULL);
				if (SUCCEEDED(hr))
				{
					TCaptureDevice device;
					_tcscpy_s(device.szDeviceName, var.bstrVal);
					refDeviceList.push_back(device);
					pBag->Release();
				}
			}
			pMoniker->Release();
		}//end while

		pEnumMoniker->Release();
		pCreateDevEnum->Release();
		return hr;
	}

	HRESULT GetCaptureFilter(REFCLSID CLSID_DeviceCategory, LPCTSTR lpFriendlyName, void** ppCapture)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);
		if (FAILED(hr)) return hr;

		IEnumMoniker* pEnumMoniker = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_DeviceCategory, &pEnumMoniker, 0);
		if (FAILED(hr))
		{
			pCreateDevEnum->Release();
			return hr;
		}
		pEnumMoniker->Reset();

		BOOL bFound = FALSE;
		IMoniker* pMoniker = NULL;
		while ((pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK )&& (!bFound))
		{
			IPropertyBag *pBag = NULL;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{
				VARIANT var = { 0 };
				var.vt = VT_BSTR;
				hr = pBag->Read(_T("FriendlyName"), &var, NULL);
				if (SUCCEEDED(hr))
				{
					if (!_tcscmp(var.bstrVal, lpFriendlyName))
					{
						hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)ppCapture);
						pMoniker->AddRef();
						bFound = TRUE;
					}
					pBag->Release();
				}
			}
			pMoniker->Release();
		}//end while
		pEnumMoniker->Release();
		pCreateDevEnum->Release();

		if (*ppCapture == NULL) return E_FAIL;
		if (FAILED(hr)) return hr;
		return bFound ? S_OK : S_FALSE;
	}

	HRESULT GetFirstCaptureFilter(REFCLSID CLSID_DeviceCategory, void** ppCapture, LPTSTR lpFriendlyName)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);
		if (FAILED(hr)) return hr;

		IEnumMoniker* pEnumMoniker = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_DeviceCategory, &pEnumMoniker, 0);
		if (FAILED(hr))
		{
			pCreateDevEnum->Release();
			return hr;
		}
		pEnumMoniker->Reset();

		BOOL bFound = FALSE;
		IMoniker* pMoniker = NULL;
		while ((pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK) && (!bFound))
		{
			IPropertyBag *pBag = NULL;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{
				VARIANT var = { 0 };
				var.vt = VT_BSTR;
				hr = pBag->Read(_T("FriendlyName"), &var, NULL);
				if (SUCCEEDED(hr))
				{
					_tcscpy_s(lpFriendlyName, MAX_PATH, var.bstrVal);
					hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)ppCapture);
					pMoniker->AddRef();
					bFound = TRUE;
					pBag->Release();
				}
			}
			pMoniker->Release();
		}//end while
		pEnumMoniker->Release();
		pCreateDevEnum->Release();

		if (*ppCapture == NULL) return E_FAIL;
		if (FAILED(hr)) return hr;
		return bFound ? S_OK : S_FALSE;
	}

	HRESULT GetAMConfigForMultiPin(IUnknown* pUnk, PIN_DIRECTION direct, IAMStreamConfig** ppConfig)
	{
		IBaseFilter* pBaseFilter = NULL;
		HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (void**)&pBaseFilter);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEnumPins = NULL;
			hr = pBaseFilter->EnumPins(&pEnumPins);
			if (SUCCEEDED(hr))
			{
				pEnumPins->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					while ((pEnumPins->Next(1, &pPin, NULL) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);
						if (SUCCEEDED(hr) && (fetchedDir == direct))
						{
							IKsPropertySet* pPS;
							hr = pPin->QueryInterface(IID_IKsPropertySet, (void**)&pPS);
							if (SUCCEEDED(hr))
							{
								GUID guid = { 0 };
								DWORD dwReturn = 0;
								hr = pPS->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, 0, 0, &guid, sizeof(guid), &dwReturn);
								if (SUCCEEDED(hr) && ::IsEqualGUID(guid, PIN_CATEGORY_CAPTURE))
								{
									hr = pPin->QueryInterface(IID_IAMStreamConfig, (void**)ppConfig);
									bFound = SUCCEEDED(hr);
								}
								pPS->Release();
							}
						}
						pPin->Release();
					}
				}
				pEnumPins->Release();
			}
			pBaseFilter->Release();
		}
		return hr;
	}

	HRESULT GetAMConfigForSinglePin(IUnknown* pUnk, PIN_DIRECTION direction, IAMStreamConfig** ppConfig)
	{
		IBaseFilter* pBaseFilter = NULL;
		HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (void**)&pBaseFilter);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEnumPins = NULL;
			hr = pBaseFilter->EnumPins(&pEnumPins);
			if (SUCCEEDED(hr))
			{
				pEnumPins->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					while (((pEnumPins->Next(1, &pPin, NULL)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);
						if (SUCCEEDED(hr) && (fetchedDir == direction))
						{
							hr = pPin->QueryInterface(IID_IAMStreamConfig, (void**)ppConfig);
							bFound = SUCCEEDED(hr);
						}
						pPin->Release();
					}
				}
				pEnumPins->Release();
			}
			pBaseFilter->Release();
		}
		return hr;
	}

	HRESULT GetPinCountForOneType(IUnknown* pUnk, PIN_DIRECTION direction, LPDWORD pPinCount)
	{
		(*pPinCount) = 0;
		IBaseFilter* pBaseFilter = NULL;
		HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBaseFilter);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEnumPins = NULL;
			hr = pBaseFilter->EnumPins(&pEnumPins);
			if (SUCCEEDED(hr))
			{
				pEnumPins->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEnumPins->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);
						if (SUCCEEDED(hr) && (fetchedDir == direction))
						{
							(*pPinCount)++;
						}
						pPin->Release();
					}
				}
				pEnumPins->Release();
			}
			pBaseFilter->Release();
		}
		return hr;
	}

	HRESULT GetAMCaptureConfig(IUnknown* pUnk, PIN_DIRECTION direction, IAMStreamConfig** ppConfig)
	{
		DWORD dwPinCount;
		HRESULT hr = GetPinCountForOneType(pUnk, direction, &dwPinCount);
		if (SUCCEEDED(hr) && dwPinCount > 0)
		{
			if (dwPinCount > 1)
			{
				hr = GetAMConfigForMultiPin(pUnk, direction, ppConfig);
			}
			else
			{
				hr = GetAMConfigForSinglePin(pUnk, direction, ppConfig);
			}
		}
		return hr;
	}

	HRESULT GetAMCaptureFormats(IAMStreamConfig* pConfig, std::vector<AM_MEDIA_TYPE>& refFormatList)
	{
		int iCount, iSize;
		HRESULT hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		if (FAILED(hr)) return hr;

		for (int i = 0; i < iCount; i++)
		{
			AM_MEDIA_TYPE* pmt = NULL;
			VIDEO_STREAM_CONFIG_CAPS vscc = { 0 };
			if (SUCCEEDED(pConfig->GetStreamCaps(i, &pmt, (BYTE*)&vscc)))
			{
				refFormatList.push_back(*pmt);
			}
		}

		return hr;
	}

	HRESULT RemoveFilter(IGraphBuilder* pGraphBuilder, IBaseFilter* pBaseFilter)
	{
		CheckPointer(pGraphBuilder, E_POINTER);
		CheckPointer(pBaseFilter, E_PENDING);

		IEnumPins *pEnumPins = NULL;
		HRESULT hr = pBaseFilter->EnumPins(&pEnumPins);
		if (FAILED(hr)) return hr;
		pEnumPins->Reset();

		IPin *pPinFrom = 0, *pPinTo = 0;
		while ((hr = pEnumPins->Next(1, &pPinFrom, NULL)) == S_OK)
		{
			if (SUCCEEDED(hr))
			{
				hr = pPinFrom->ConnectedTo(&pPinTo);
				if (SUCCEEDED(hr))
				{
					PIN_INFO info;
					hr = pPinTo->QueryPinInfo(&info);
					if (SUCCEEDED(hr))	
					{
						if (info.dir == PINDIR_INPUT)
						{
							RemoveFilter(pGraphBuilder, info.pFilter);
							pGraphBuilder->Disconnect(pPinTo);
							pGraphBuilder->Disconnect(pPinFrom);
							pGraphBuilder->RemoveFilter(info.pFilter);
						}
						info.pFilter->Release();
					}
					pPinTo->Release();
				}
				pPinFrom->Release();
			}
		}
		pEnumPins->Release();
		return S_OK;
	}
}
