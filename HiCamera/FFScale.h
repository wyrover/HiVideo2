#ifndef __E_FFSCALE_H__
#define __E_FFSCALE_H__
extern "C" {
#include "libswscale\swscale.h"
}

namespace e
{
	//Scale算法
	enum ScaleAlgorithm
	{
		SWS_SA_FAST_BILINEAR = 0x1,
		SWS_SA_BILINEAR = 0x2,
		SWS_SA_BICUBIC = 0x4,
		SWS_SA_X = 0x8,
		SWS_SA_POINT = 0x10,
		SWS_SA_AREA = 0x20,
		SWS_SA_BICUBLIN = 0x40,
		SWS_SA_GAUSS = 0x80,
		SWS_SA_SINC = 0x100,
		SWS_SA_LANCZOS = 0x200,
		SWS_SA_SPLINE = 0x400,
	};

	//视频图像格式
	enum PictureFormat
	{
		SWS_FMT_NONE = PIX_FMT_NONE,
		SWS_FMT_YUV420P = PIX_FMT_YUV420P,
		SWS_FMT_RGB24 = PIX_FMT_RGB24,
		SWS_FMT_BGR24 = PIX_FMT_BGR24,
		SWS_FMT_ARGB = PIX_FMT_ARGB,
		SWS_FMT_RGBA = PIX_FMT_RGBA,
		SWS_FMT_ABGR = PIX_FMT_ABGR,
		SWS_FMT_BGRA = PIX_FMT_BGRA,
		SWS_FMT_YUV422 = PIX_FMT_YUYV422,
		SWS_FMT_UVVY422 = PIX_FMT_UYVY422
	};

	class CFFScale
	{
	public:
		CFFScale(void);
		virtual ~CFFScale(void);

		void SetAttribute(PictureFormat eSourceFormat
			, PictureFormat eTargetFormat
			, ScaleAlgorithm eAlgorithm);

		bool Scale(void* pSource
			, int nSourceWidth
			, int nSourceHeight
			, int nSourcePitch
			, void* pTarget
			, int nTargetWidth
			, int nTargetHeight
			, int nTargetPitch);
	private:
		bool Initialize(void);
		bool IsNeedReset(int nSourceWidth
			, int nSourceHeight
			, int nSourcePicth
			, int nTargetWidth
			, int nTargetHeight
			, int nTargetPicth);
		void Cleanup(void);
	protected:
		int m_nSourceWidth;			//源图像宽高
		int m_nSourceHeight;
		int m_nSourcePicth;			//源图像第一行数据的长度
		int m_nSourceSlice[4];		//源图像各分量数据起始地址偏移
		int m_nSourceStride[4];		//源图像各分量一行数据的长度

		int m_nTargetWidth;			//目标图像宽高
		int m_nTargetHeight;
		int m_nTargetPicth;			//目标图像第一行数据的长度
		int m_nTargetSlice[4];			//目标图像各分量数据起始地址偏移
		int m_nTargetStride[4];		//目标图像各分量一行数据的长度
		
		PictureFormat m_eSourceFormat;
		PictureFormat m_eTargetFormat;
		ScaleAlgorithm m_eAlgorithm;
		SwsContext* m_pContext;
	};
}

#endif