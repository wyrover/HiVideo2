#pragma once
#include "IBaseEffect.h"
namespace e
{
	class CMemBlock;

	class CImageEffectConfig 
		: public IEffectConfig
	{
	public:
		virtual void SetConfig(void* pConfig, int nSize);
		virtual void GetConfig(void** ppConfig);
	protected:
		float m_fSigma;
		float m_fAlpha;
	};

	class CImageEffect 
		: public IBaseEffect
	{
	public:
		CImageEffect(void);
		virtual ~CImageEffect(void);
		virtual void SetType(int nType);
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
		IEffectConfig* m_pConfig;
		IBaseEffect* m_pImageBlur;
		CMemBlock* m_pBlockTemp;
	};
}
