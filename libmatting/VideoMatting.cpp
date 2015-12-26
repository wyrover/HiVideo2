#include "stdafx.h"
#include "VideoMatting.h"
#include "Bitmap.h"
#include "ImageFilter.h"
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

		m_pFilter = new CImageFilter();
		assert(m_pFilter);

		m_nOptions = 0;
		m_nThreshold = 15;
		m_nBlockSize = 8;

		//SetOption(OPTION_GRAY, true);
		SetOption(OPTION_SMOOTH, true);
		//SetOption(OPTION_SPLITBLOCK, true);
		
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

	bool CVideoMatting::InitializeImage(int nWidth, int nHeight, int nBitCount)
	{
		if (!GetBitmap(real_bg_image)->Create(nWidth, nHeight, 32, NULL), true)
			return false;
		if (!GetBitmap(current_image)->Create(nWidth, nHeight, 32, NULL), true)
			return false;
		if (!GetBitmap(current_trimap)->Create(nWidth, nHeight, 32, NULL), true)
			return false;
		if (!GetBitmap(current_alpha)->Create(nWidth, nHeight, 8, NULL), true)
			return false;
		m_nState &= STATE_MATTING;
		return true;
	}

	void CVideoMatting::SetRealBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
// 		TCHAR szPath[MAX_PATH] = { 0 };
// 		_stprintf_s(szPath, _T("f:\\bk%u.bmp"), GetTickCount());
// 		CBitmap::Save(szPath, nWidth, nHeight, nBitCount, pData);

		StoreImage(real_bg_image, pData, nSize, nWidth, nHeight, nBitCount);
		PreprocessBackground();
		m_nState &= STATE_SET_RBG;
	}

	void CVideoMatting::SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		GetBitmap(virtual_bg_image)->Store(pData, nWidth, nHeight, nBitCount);
		m_nState &= STATE_SET_VBG;
	}

	bool CVideoMatting::StoreImage(ImageType eType, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		CBitmap* pImage = GetBitmap(eType);
		assert(pImage);
		return pImage->Store(pData, nWidth, nHeight, nBitCount) ? true : false;
	}

	void CVideoMatting::SetMattingThreshold(int nThreshold)
	{
		m_nThreshold = nThreshold;
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
		CBitmap* pBG = GetBitmap(real_bg_image);
		//对背景图片进行预处理
		PreprocessImage(pBG);
		//转化为灰度图并保存
		m_pFilter->ConvertGray(pBG);
	}

	void CVideoMatting::PreprocessForeground(void)
	{
		CBitmap* pFG = GetBitmap(current_image);
		//对前景图片进行预处理
		PreprocessImage(pFG);
		//转化为灰度图并保存
		m_pFilter->ConvertGray(pFG);
	}

	template<class T> T square(T x)
	{
		return x * x;
	}

	template<class T> T diff(T&r0, T&b0, T&g0, T&a0, T&r1, T&b1, T&g1, T&a1)
	{
		T d0 = abs(r0-r1) + abs(b0-b1) + abs(g0 - g1);
		T d1 = abs(a0 - a1);
		//T dx = sqrt(square(r0 - r1) + square(b0 - b1) + square(g0 - g1));
		return (d0 + d1)/4;
	}

	//初步计算mask
	void CVideoMatting::CalcTrimap(void* pTrimap, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount)
	{
		int nLineSize = WidthBytes(nWidth * nBitCount);
		int nPixelSize = nBitCount >> 3;

		for (int y = 0; y < nHeight; y++)
		{
			uint8* pSrc0 = (uint8*)pBG + y * nLineSize;
			uint8* pSrc1 = (uint8*)pFG + y * nLineSize;
			uint8* pDest = (uint8*)pTrimap + y * nLineSize;
			for (int x = 0; x < nWidth; x++)
			{
				pDest[0] = pDest[1] = pDest[2] = diff(pSrc0[0], pSrc0[1], pSrc0[2], pSrc0[3], pSrc1[0], pSrc1[1], pSrc1[2], pSrc1[3]);
				pSrc0 += nPixelSize;
				pSrc1 += nPixelSize;
				pDest += nPixelSize;
			}
		}
	}

	void CVideoMatting::CalcMatting(void* pData, void* pAlpha, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes0 = WidthBytes(nWidth * nBitCount);
		int nLineBytes1 = WidthBytes(nWidth * 8);

		for (int y = 0; y < nHeight; y++)
		{
			uint8* pDest = (uint8*)pData + y * nLineBytes0;
			uint8* pFlag = (uint8*)pAlpha + y * nLineBytes1;
			for (int x = 0; x < nWidth; x++)
			{
				if (!pFlag[x])
				{
					pDest[0] = 0;
					pDest[1] = 0xff;
					pDest[2] = 0;
					pDest[3] = 0xff;
				}
				pDest += nBitCount >> 3;
			}
		}
	}

	void CVideoMatting::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//保存一个副本
		StoreImage(current_image, pData, nSize, nWidth, nHeight, nBitCount);
		//预处理
		PreprocessForeground();
		//计算trimap
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pFG = GetBitmap(current_image);
		CalcTrimap(pData, pBG->GetBits(), pFG->GetBits(), nWidth, nHeight, nBitCount);
		//计算alpha
		
		//抠图
	}

	void CVideoMatting::CompareImage(CBitmap* pDest, CBitmap* pSrc0, CBitmap* pSrc1)
	{
		int nWidth = pSrc0->Width();
		int nHeight = pSrc0->Height();
		int nBitCount = pSrc0->BitCount();

		int nPixelSize = nBitCount >> 3;
		int nLineSize = WidthBytes(nWidth*nBitCount);
		for (int y = 0; y < nHeight; y++)
		{
			uint8* p0 = pSrc0->GetBits(0, y);
			uint8* p1 = pSrc1->GetBits(0, y);
			uint8* p2 = pDest->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int B0 = p0[0], G0 = p0[1], R0 = p0[2], A0 = p0[3];
				int B1 = p1[0], G1 = p1[1], R1 = p1[2], A1 = p1[3];
				p2[0] = abs(B0 - B1) + abs(G0 + G1) + abs(R0 + R1);
				p2[1] = abs(A0 - A1);

				p0 += nPixelSize;
				p1 += nPixelSize;
				p2 += nPixelSize;
			}
		}
	}

	void CVideoMatting::OnSampleSave(const TCHAR* pFileName)
	{
		//GetBitmap(current_trimap)->Save(pFileName);
		CBitmap* pDiff = GetBitmap(current_trimap);
		CBitmap* pBG = GetBitmap(real_bg_image);
		CBitmap* pFG = GetBitmap(current_image);
		CompareImage(pDiff, pBG, pFG);

		int nWidth = pDiff->Width();
		int nHeight = pDiff->Height();
		int nBitCount = pDiff->BitCount();

		int nBufferSize = nWidth * nHeight * 32;
		char* pBuffer = new char[nBufferSize];
		assert(pBuffer);

		int nOffset = 0;
		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = pDiff->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int nValue = p[0];
				nOffset += sprintf_s(pBuffer+nOffset, nBufferSize-nOffset, "%03d ", nValue);
				p += nBitCount >> 3;
			}
			nOffset += sprintf_s(pBuffer + nOffset, nBufferSize-nOffset, "\r\n");
		}

		nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, 
			"----------------------------------------------------------------------------------------------------------------\r\n");

		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = pDiff->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int nValue = p[1];
				nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "%03d ", nValue);
				p += nBitCount >> 3;
			}
			nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "\r\n");
		}

		do 
		{
			HANDLE hFile = CreateFile(pFileName
				, GENERIC_WRITE
				, 0
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_NORMAL
				, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				Log(_T("open file failed : %u, %s"), GetLastError(), pFileName);
				break;
			}

			DWORD dwWroteBytes = 0;
			WriteFile(hFile, pBuffer, strlen(pBuffer), &dwWroteBytes, NULL);
			FlushFileBuffers(hFile);
			CloseHandle(hFile);
			
		} while (0);

		if (pBuffer) delete[] pBuffer;
	}

	void CVideoMatting::Reset(void)
	{
		m_nState = 0;
	}

	void CVideoMatting::Clean(void)
	{
		for (int i = 0; i < m_nNumberOfImage; i++)
		{
			SafeDelete(&m_pImages[i]);
		}
		SafeDelete(&m_pImages);
	}
}