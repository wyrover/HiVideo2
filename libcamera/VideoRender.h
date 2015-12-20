#pragma once
#include "GUIDDefs.h"
#include "IVideoDevice.h"
#include "FFScale.h"
namespace e
{
	class CVideoRenderer : public CBaseRenderer
	{
	public:
		CVideoRenderer(LPCTSTR lpName, LPUNKNOWN lpUnk, HRESULT* phr);
		virtual ~CVideoRenderer(void);
	public:
		void SetSampleCallback(IVideoSample* pSampleCallback);
		void SetOutputFormat(int nWidth, int nHeight);
		 void OnReceiveFirstSample(IMediaSample* sample){};
		HRESULT CheckMediaType(const CMediaType* pMediaType);
		HRESULT DoRenderSample(IMediaSample *pMediaSample);
	protected:
		HRESULT GetVideoType(REFCLSID subtype, VideoType& eType);
		HRESULT CreateRGBABuffer(int nWidth, int nHeight, int nBitCount);
	protected:
		CMediaType m_mt;
		CCritSec m_cStateLock;
		//capture sample config
		int m_nWidth;
		int m_nHeight;
		int m_nBitCount;
		VideoType m_eVideoType;
		IVideoSample* m_pSampleCallback;
		//samples out process
		CFFScale m_ffScaler;
		int  m_nOutWidth;
		int m_nOutHeight;
		int m_nOutBitCount;
		BYTE* m_pOutBuffer;
	};
}
