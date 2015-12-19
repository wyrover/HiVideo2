#include "stdafx.h"
#include "VideoMatting.h"
#include "Bitmap.h"

namespace e
{
	CVideoMatting::CVideoMatting()
	{
		m_pBitmap = new CBitmap();
	}

	CVideoMatting::~CVideoMatting()
	{
		if (m_pBitmap) delete m_pBitmap;
	}
}