//#include "stdafx.h"
#include "FFScale.h"
#pragma comment(lib, "libgcc.lib")
#pragma comment(lib, "libmingwex.lib")
#pragma comment(lib, "libavutil.lib")
#pragma comment(lib, "libswscale.lib")

namespace e
{
	typedef unsigned char byte;

	CFFScale::CFFScale(void)
	{
		m_pContext = 0;
		m_eSourceFormat = SWS_FMT_NONE;
		m_eTargetFormat = SWS_FMT_NONE;
		m_eAlgorithm = SWS_SA_FAST_BILINEAR;

		m_nSourceWidth = 0;
		m_nSourceHeight = 0;
		m_nSourcePicth = 0;
		m_nTargetWidth = 0;
		m_nTargetHeight = 0;
		m_nTargetPicth = 0;

		for (int i = 0; i < 4; i++)
		{
			m_nSourceSlice[i] = -1;
			m_nSourceStride[i] = 0;
			m_nTargetSlice[i] = -1;
			m_nTargetStride[i] = 0;
		}
	}

	CFFScale::~CFFScale(void)
	{
		Cleanup();
	}

	bool CFFScale::Initialize(void)
	{
		if (m_eSourceFormat == SWS_FMT_NONE || m_eTargetFormat == SWS_FMT_NONE)
		{
			return false;
		}

		if (m_pContext != 0)
		{
			Cleanup();
		}

		m_pContext = sws_getContext(m_nSourceWidth
			, m_nSourceHeight
			, (PixelFormat)m_eSourceFormat
			, m_nTargetWidth
			, m_nTargetHeight
			, (PixelFormat)m_eTargetFormat
			, m_eAlgorithm
			, 0, 0, 0);
		if (m_pContext == 0) return false;

		//初始化源Slice和Stride
		if (m_eSourceFormat == SWS_FMT_YUV420P)
		{
			m_nSourceSlice[0] = 0;
			m_nSourceSlice[1] = m_nSourceWidth * m_nSourceHeight;
			m_nSourceSlice[2] = m_nSourceWidth * m_nSourceHeight * 5 / 4;
			m_nSourceSlice[3] = -1;

			m_nSourceStride[0] = m_nSourceWidth;
			m_nSourceStride[1] = m_nSourceWidth / 2;
			m_nSourceStride[2] = m_nSourceWidth / 2;
			m_nSourceStride[3] = 0;

		}
		else
		{
			m_nSourceSlice[0] = 0;
			m_nSourceSlice[1] = -1;
			m_nSourceSlice[2] = -1;
			m_nSourceSlice[3] = -1;

			m_nSourceStride[0] = m_nSourcePicth;
			m_nSourceStride[1] = 0;
			m_nSourceStride[2] = 0;
			m_nSourceStride[3] = 0;
		}

		//初始化目标Slice和Stride
		if (m_eTargetFormat == SWS_FMT_YUV420P)
		{
			m_nTargetSlice[0] = 0;
			m_nTargetSlice[1] = m_nTargetWidth * m_nTargetHeight;
			m_nTargetSlice[2] = m_nTargetWidth * m_nTargetHeight * 5 / 4;
			m_nTargetSlice[3] = -1;

			m_nTargetStride[0] = m_nTargetWidth;
			m_nTargetStride[1] = m_nTargetWidth / 2;
			m_nTargetStride[2] = m_nTargetWidth / 2;
			m_nTargetStride[3] = 0;
		}
		else
		{
			m_nTargetSlice[0] = 0;
			m_nTargetSlice[1] = -1;
			m_nTargetSlice[2] = -1;
			m_nTargetSlice[3] = -1;

			m_nTargetStride[0] = m_nTargetPicth;
			m_nTargetStride[1] = 0;
			m_nTargetStride[2] = 0;
			m_nTargetStride[3] = 0;
		}

		return true;
	}

	bool CFFScale::IsNeedReset(int nSourceWidth
		, int nSourceHeight
		, int nSourcePicth
		, int nTargetWidth
		, int nTargetHeight
		, int nTargetPicth)
	{
		return m_nSourceWidth!=nSourceWidth || m_nSourceHeight != nSourceHeight ||
			m_nSourcePicth != nSourcePicth || m_nTargetWidth != nTargetWidth ||
			m_nTargetHeight != nTargetHeight || m_nTargetPicth != nTargetPicth;
	}

	void CFFScale::SetAttribute(PictureFormat eSourceFormat
		, PictureFormat eTargetFormat
		, ScaleAlgorithm eAlgorithm)
	{
		if (m_eSourceFormat != eSourceFormat ||
			m_eTargetFormat != eTargetFormat ||
			m_eAlgorithm != eAlgorithm)
		{
			m_eSourceFormat = eSourceFormat;
			m_eTargetFormat = eTargetFormat;
			m_eAlgorithm = eAlgorithm;

			Cleanup();
		}
	}

	bool CFFScale::Scale(void* pSource
		, int nSourceWidth
		, int nSourceHeight
		, int nSourcePitch
		, void* pTarget
		, int nTargetWidth
		, int nTargetHeight
		, int nTargetPitch)
	{
		if (IsNeedReset(nSourceWidth, nSourceHeight, nSourcePitch
			, nTargetWidth, nTargetHeight, nTargetPitch))
		{
			m_nSourceWidth = nSourceWidth;
			m_nSourceHeight = nSourceHeight;
			m_nSourcePicth = nSourcePitch;
			m_nTargetWidth = nTargetWidth;
			m_nTargetHeight = nTargetHeight;
			m_nTargetPicth = nTargetPitch;
			Cleanup();
		}

		if (m_pContext == 0 && Initialize())
		{
			return false;
		}

		//真正的Scale操作
		byte *pSourceSlice[4], *pTargetSlice[4];
		for (int i = 0; i < 4; i++)
		{
			pSourceSlice[i] = m_nSourceSlice[i] < 0 ? 0 : ((byte*)pSource + m_nSourceSlice[i]);
			pTargetSlice[i] = m_nTargetSlice[i] < 0 ? 0 : ((byte*)pTarget + m_nTargetSlice[i]);
		}

		int nRet = sws_scale(m_pContext,
			pSourceSlice,
			m_nSourceStride,
			0,
			m_nSourceHeight,
			pTargetSlice,
			m_nTargetStride);

		return nRet == m_nTargetHeight;
	}

	void CFFScale::Cleanup(void)
	{
		if (m_pContext)
		{
			sws_freeContext(m_pContext);
			m_pContext = 0;
		}
	}
}