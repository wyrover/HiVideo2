#pragma once
namespace e
{
	class IEffectConfig
	{
	public:
		virtual void SetConfig(void* pConfig, int nSize) = 0;
		virtual void GetConfig(void** ppConfig) = 0;
	};

	class IBaseEffect
	{
	public:
		virtual void SetType(int nType) = 0;
		virtual void SetConfig(IEffectConfig* pConfig) = 0;
		virtual void OnSampleProc(void* pData
			, int nSize
			, int nWidth
			, int nHeight
			, int nBitCount) = 0;
	};
}