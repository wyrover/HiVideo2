#ifndef __E_VIDEODEVICE_H__
#define __E_VIDEODEVICE_H__
#include "GUIDDefs.h"
#include "IVideoDevice.h"
namespace e
{
	class CVideoDevice
	{
	public:
		CVideoDevice(HRESULT* phr);
		virtual ~CVideoDevice(void);
	public:
		//Enum Device Config
		HRESULT GetCaptureDeviceNames(IVideoDevice* pCallback);
		HRESULT GetCaptureDeviceFormats(LPCTSTR lpDeviceName, IVideoFormat* pCallback);
		//Get Current Device info
		HRESULT GetCurrentDeviceFormats(IVideoFormat* pCallback);
		HRESULT GetCurrentOutputDevice(LPTSTR lpDeviceName);
		HRESULT GetCurrentOutputFormat(LPTSTR lpDeviceFormat);
		HRESULT SetCurrentOutputFormat(int nWidth, int nHeight, VideoType eType = YUY2);
		//device operation
		HRESULT Create(LPCTSTR lpDeviceName, IVideoSample* pCallback);
		HRESULT Start(void);
		HRESULT Pause(void);
		HRESULT Stop(void);
		HRESULT Destroy(void);
	protected:
		HRESULT CreateCapture(LPCTSTR lpDeviceName, IBaseFilter** ppCapture);
		HRESULT CreateDecoder(VideoType eType, IBaseFilter** ppDecoder);
		HRESULT CreateRender(IVideoSample* pCallback, IBaseFilter** ppRender);
		HRESULT SetDefaultConfig(void);
		HRESULT GetSubtype(VideoType eType, GUID& subtype);
		HRESULT GetSubtypeName(REFGUID subtype, LPTSTR lpTypeName);
		HRESULT GetVideoTypeName(VideoType eType, LPTSTR lpTypeName);
		HRESULT SetCaptureConfig(IBaseFilter* pCapture, int nWidth, int nHeight);
		enum DeviceState{ Unbuilde, Builded, Stopped, Paused, Running };
		DeviceState GetState(void) const { return m_eState; }
	protected:
		DeviceState m_eState;
		//directshow filter
		TCHAR m_szDeviceName[MAX_PATH];
		IGraphBuilder* m_pGraphBuilder;
		ICaptureGraphBuilder2* m_pCaptureGraphBuilder2;
		IBaseFilter* m_pVideoCapture;
		IBaseFilter* m_pVideoDecoder;
		IBaseFilter* m_pVideoRender;
		IMediaControl* m_pMediaControl;
		//video config
		int m_nVideoWidth;
		int m_nVideoHeight;
		VideoType m_eVideoType;
	};
}
#endif
