#pragma once
#include "IBaseEffect.h"
namespace e
{
	class CImageBlur :
		public IBaseEffect
	{
	public:
		CImageBlur(void);
		virtual ~CImageBlur(void);
		virtual void SetConfig(IEffectConfig* pConfig);
		virtual void OnSampleProc(void* pData
			, int nSize
			, int nWidth
			, int nHeight
			, int nBitCount);
	protected:
		bool Init(int nSize);
		void Clean(void);
	protected:
		float m_fSigma;

		void* m_pBuffer;
		int  m_nBufferSize;
	};
}
