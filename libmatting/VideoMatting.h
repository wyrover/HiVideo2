#pragma once

namespace e
{
	class CBitmap;
	class CVideoMatting
	{
	public:
		CVideoMatting();
		virtual ~CVideoMatting();
	protected:
		int x;
		CBitmap* m_pBitmap;
	};
}
