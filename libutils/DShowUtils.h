#pragma once
#include <wtypes.h>
#include <strmif.h>
#include <uuids.h>
#include <vector>

namespace e
{
	struct TCaptureDevice{
		TCHAR szDeviceName[MAX_PATH];
	};

	HRESULT EnumCaptures(REFCLSID CLSID_DeviceCategory, std::vector<TCaptureDevice>& refDeviceList);
	HRESULT GetCaptureFilter(REFCLSID CLSID_DeviceCategory, LPCTSTR lpFriendlyName, void** ppCapture);
	HRESULT GetFirstCaptureFilter(REFCLSID CLSID_DeviceCategory, void** ppCapture, LPTSTR lpFriendlyName);
	HRESULT GetAMConfigForMultiPin(IUnknown* pUnk, PIN_DIRECTION direction, IAMStreamConfig** ppConfig);
	HRESULT GetAMConfigForSinglePin(IUnknown* pUnk, PIN_DIRECTION direction, IAMStreamConfig** ppConfig);
	HRESULT GetPinCountForOneType(IUnknown* pUnk, PIN_DIRECTION direction, LPDWORD pPinCount);
	HRESULT GetAMCaptureConfig(IUnknown* pUnk, PIN_DIRECTION direction, IAMStreamConfig** ppConfig);
	HRESULT GetAMCaptureFormats(IAMStreamConfig* pConfig, std::vector<AM_MEDIA_TYPE>& refFormatList);

	template<typename T> void SafeRelease(T** pp)
	{
		if (*pp)
		{
			(*pp)->Release();
		}
		*pp = NULL;
	}
}

