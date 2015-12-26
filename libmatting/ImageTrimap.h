#pragma once

namespace e
{
	class CImageTrimap
	{
	public:
		CImageTrimap(void);
		virtual ~CImageTrimap(void);
		void CalcTrimap(void* pTrimap, void* pData, int nWidth, int nHeight, int nBitCount);
	protected:

	};
}

