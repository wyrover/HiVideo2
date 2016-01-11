#include "stdafx.h"
#include "VideoMatting.h"
#include "Bitmap.h"
#include "Filter.h"
#include "Differen.h"
#include "ImageDumper.h"
#include "ColorSpace.h"

namespace e
{
	CVideoMatting::CVideoMatting(void)
		: m_nNumberOfImage(_max_image_count)
	{
		m_nState = STATE_RESET;
		m_pImages = new CBitmap*[m_nNumberOfImage];
		assert(m_pImages);
		for (int i = 0; i < m_nNumberOfImage; i++)
		{
			m_pImages[i] = new CBitmap();
			assert(m_pImages[i]);
		}

		m_pDiffer = new CDifferen();
		assert(m_pDiffer);

		m_pFilter = new CImageFilter();
		assert(m_pFilter);
		m_pDumper = new CImageDumper();
		assert(m_pDumper);

		m_nOptions = 0;
		m_nThreshold[0] = 15;
		m_nThreshold[1] = 35;
		m_nBlockSize = 4;

		//SetOption(OPTION_GRAY, true);
		//SetOption(OPTION_SMOOTH, true);
		SetOption(OPTION_SPLITBLOCK, true);
	}

	CVideoMatting::~CVideoMatting(void)
	{
		Clean();
	}

	CBitmap* CVideoMatting::GetBitmap(ImageType eType) const
	{
		return m_pImages[eType];
	}

	void CVideoMatting::SetOption(int nOption, bool bAdd)
	{
		if (bAdd)
			m_nOptions |= nOption;
		else
			m_nOptions &= ~nOption;
	}

	bool CVideoMatting::IsReadyMatting(void) const
	{
		return (m_nState & STATE_INITIALIZE)
			&& (m_nState & STATE_SET_RBG)
			&& (m_nState & STATE_SET_VBG);
	}

	bool CVideoMatting::Initialize(int nWidth, int nHeight, int nBitCount)
	{
		if (!GetBitmap(real_bg_image)->Create(nWidth, nHeight, 32, NULL, true))
			return false;
		if (!GetBitmap(current_image)->Create(nWidth, nHeight, 32, NULL, true))
			return false;
		if (!GetBitmap(current_trimap)->Create(nWidth, nHeight, 32, NULL, true))
			return false;
		if (!GetBitmap(current_alpha)->Create(nWidth, nHeight, 8, NULL, true))
			return false;
		//graph
		if (!GetBitmap(real_bg_graph)->Create(nWidth / m_nBlockSize, nHeight / m_nBlockSize, 8, NULL, true))
			return false;
		if (!GetBitmap(current_graph)->Create(nWidth / m_nBlockSize, nHeight / m_nBlockSize, 8, NULL, true))
			return false;
		if (!GetBitmap(current_edge)->Create(nWidth / m_nBlockSize, nHeight / m_nBlockSize, 8, NULL, true))
			return false;
		m_nState &= STATE_MATTING;
		return true;
	}

	void CVideoMatting::SetBackground(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
#ifndef _DEBUG 
		CBitmap::Save(_T("f:\\bg.bmp"), nWidth, nHeight, nBitCount, pData);
#endif
		StoreImage(real_bg_image, pData, nSize, nWidth, nHeight, nBitCount);
		StoreImage(real_bg_block, pData, nSize, nWidth, nHeight, nBitCount);
		PreprocessBackground();
		m_nState &= STATE_SET_RBG;
	}

	void CVideoMatting::SetVirtualBackground(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//GetBitmap(virtual_bg_image)->Store(pData, nWidth, nHeight, nBitCount);
		m_nState &= STATE_SET_VBG;
	}

	bool CVideoMatting::StoreImage(ImageType eType, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		CBitmap* pImage = GetBitmap(eType);
		assert(pImage);
		return pImage->Store(pData, nWidth, nHeight, nBitCount) ? true : false;
	}

	void CVideoMatting::SetMattingThreshold(int nIndex, int nThreshold)
	{
		m_nThreshold[nIndex] = nThreshold;
		if (nIndex == 0)
			m_pDiffer->SetThreshold(nThreshold);
	}

	void CVideoMatting::RemoveNoiscEnable(bool bEnable)
	{
		SetOption(OPTION_GRAPHNOISE, bEnable);
	}

	void CVideoMatting::PreprocessImage(CBitmap* pImage)
	{
		void* pData = pImage->GetBits();
		int nSize = pImage->Size();
		int nWidth = pImage->Width();
		int nHeight = pImage->Height();
		int nBitCount = pImage->BitCount();

		//平滑去噪
		if (m_nOptions & OPTION_SMOOTH){
			m_pFilter->Smooth(pData, nSize, nWidth, nHeight, nBitCount);
		}

		//转成灰度图
		if (m_nOptions & OPTION_GRAY){
			m_pFilter->ConvertGray(pData, nSize, nWidth, nHeight, nBitCount);
		}

		//分割成块
		if (m_nOptions & OPTION_SPLITBLOCK){
			m_pFilter->SplitBlock(pData, nSize, nWidth, nHeight, nBitCount, m_nBlockSize);
		}
	}

	void CVideoMatting::PreprocessBackground(void)
	{
		CBitmap* pBG = GetBitmap(real_bg_block);
		//对背景图片进行预处理
		PreprocessImage(pBG);
		//转化为灰度图并保存到alpha通道
		m_pFilter->ConvertGray(pBG);
	}

	void CVideoMatting::PreprocessForeground(void)
	{
		CBitmap* pFG = GetBitmap(current_block);
		//对前景图片进行预处理
		PreprocessImage(pFG);
		//转化为灰度图并保存到alpha通道
		m_pFilter->ConvertGray(pFG);
	}

	inline int square(int x)
	{
		return x * x;
	}

	inline int distance(int r0, int b0, int g0, int a0, int r1, int b1, int g1, int a1)
	{
		//int d1 = abs(a0 - a1);
		//return abs(r0 - r1) + abs(b0 - b1) + abs(g0 - g1);
		return (int)sqrt(square(r0 - r1) + square(b0 - b1) + square(g0 - g1));
	}

	void CVideoMatting::CalcDiff(void* pDiff, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount)
	{
		int nLineSize = WidthBytes(nWidth * nBitCount);
		int nPixelSize = nBitCount >> 3;

		int t0 = m_nThreshold[0], t1 = m_nThreshold[1];

		for (int y = 0; y < nHeight; y++)
		{
			uint8* pSrc0 = (uint8*)pBG + y * nLineSize;
			uint8* pSrc1 = (uint8*)pFG + y * nLineSize;
			uint8* pDest = (uint8*)pDiff + y * nLineSize;

			for (int x = 0; x < nWidth; x++)
			{
				int dx = distance(pSrc0[0], pSrc0[1], pSrc0[2], pSrc0[3], pSrc1[0], pSrc1[1], pSrc1[2], pSrc1[3]);
				//dx = (dx < t0) ? 0 : ((dx < t1) ? 128 : 255);
				dx = (dx < t0) ? 0 : 255;
				pDest[0] = pDest[1] = pDest[2] = dx;
				pSrc0 += nPixelSize;
				pSrc1 += nPixelSize;
				pDest += nPixelSize;
			}
		}
	}

	void CVideoMatting::CalcAlpha(void* pAlpha
		, void* pTrimap
		, void* pBG
		, void* pFG
		, int nWidth
		, int nHeight
		, int nBitCount)
	{
	
	}

	void CVideoMatting::CalcMatting(void* pData, void* pAlpha, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes0 = WidthBytes(nWidth * nBitCount);
		int nLineBytes1 = WidthBytes(nWidth * nBitCount);

		int t = m_nThreshold[0];
		for (int y = 0; y < nHeight; y++)
		{
			uint8* pDest = (uint8*)pData + y * nLineBytes0;
			uint8* pFlag = (uint8*)pAlpha + y * nLineBytes1;
			for (int x = 0; x < nWidth; x++)
			{
				if (pFlag[0] < t)
				{
					pDest[0] = 0;
					pDest[1] = 0xff;
					pDest[2] = 0;
					pDest[3] = 0xff;
				}
				pDest += nBitCount >> 3;
				pFlag += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//保存一个副本
		StoreImage(current_image, pData, nSize, nWidth, nHeight, nBitCount);
		StoreImage(current_block, pData, nSize, nWidth, nHeight, nBitCount);
		//预处理
		PreprocessForeground();
		{
			CBitmap* pBG = GetBitmap(real_bg_block);
			CBitmap* pFG = GetBitmap(current_block);
			CBitmap* pAlpha = GetBitmap(current_alpha);
			//m_pDiffer->Differen(pAlpha, pBG, pFG);
			//pAlpha->Save(_T("f:\\diff.bmp"));
			m_pDiffer->Differen(pData, pBG->GetBits(), pData, nWidth, nHeight, nBitCount);
			return;
		}
		//计算trimap，黑点表示用背景，白点表示用前景
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pFG = GetBitmap(current_block);
		CBitmap* pTG = GetBitmap(current_trimap);
		



		CalcDiff(pTG->GetBits(), pBG->GetBits(), pFG->GetBits(), nWidth, nHeight, nBitCount);
		//计算alpha
		if (m_nOptions & OPTION_GRAPHNOISE)
		{
			CBitmap* pGraph = GetBitmap(current_graph);
			m_pFilter->Block2Graph(pGraph, pTG, m_nBlockSize);
			m_pFilter->RemoveNoise(pGraph);
			//m_pFilter->Graph2Block(pTG, pGraph, m_nBlockSize);

			CBitmap* pEdge = GetBitmap(current_edge);
			memset(pEdge->GetBits(), 0, pEdge->Size());
			m_pFilter->CalcEdge(pGraph, pEdge);
			pEdge->Save(_T("f:\\edge.bmp"));

			m_pFilter->CalcMerge(pGraph, pGraph, pEdge);
			pGraph->Save(_T("f:\\trimap.bmp"));
			m_pFilter->Graph2Block(pTG, pGraph, m_nBlockSize);

			CBitmap* pAlpha = GetBitmap(current_alpha);
			pBG = GetBitmap(real_bg_image);
			pFG = GetBitmap(current_image);
			m_pFilter->CalcAlpha(pAlpha, pTG, pBG, pFG);
			pAlpha->Save(_T("f:\\alpha.bmp"));
		}
		//抠图处理
		CalcMatting(pData, pTG->GetBits(), nWidth, nHeight, nBitCount);
	}

	void CVideoMatting::OnSampleSave(const TCHAR* pFileName)
	{
		//GetBitmap(current_trimap)->Save(pFileName);
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pFG = GetBitmap(current_image);
		pFG->Save(pFileName);
		//compare images
		//m_pDumper->Difference(pFileName, pBG, pFG);
	}

	void CVideoMatting::Reset(void)
	{
		m_nState = 0;
	}

	void CVideoMatting::Clean(void)
	{
		SafeDelete(&m_pDiffer);
		SafeDelete(&m_pFilter);
		SafeDelete(&m_pDumper);
		for (int i = 0; i < m_nNumberOfImage; i++)
		{
			SafeDelete(&m_pImages[i]);
		}
		SafeDelete(&m_pImages);
	}
}