#ifndef __E_IVIDEODEVICE_H__
#define __E_IVIDEODEVICE_H__
#include <Unknwn.h>
namespace e
{
	const int kOutVideoWidth = 640;
	const int kOutVideoHeight = 480;
	
	typedef enum{ 
		UNKNOWN,  RGB24, RGB32,IMC1, IMC2, IMC3, IMC4, 
		IYUV, YUYV, YVYU, UYVY, YV12, YUY2, I420, MJPG
	}VideoType;

	interface IVideoDevice
	{
		virtual void OnAddDevice(const TCHAR* pszDeviceName) = 0;
	};

	interface IVideoFormat
	{
		virtual void OnAddFormat(const TCHAR* pszFormat) = 0;
	};

	interface IVideoSample
	{
		virtual void OnAddSample(void* pData
			, int nSize
			, int nWidth
			, int nHeight
			, int nBitCount) = 0;
	};

}

#endif