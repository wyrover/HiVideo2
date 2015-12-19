#pragma once
namespace e
{
	class IBaseEffect
	{
	public:
		virtual void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount) = 0;
	};
}