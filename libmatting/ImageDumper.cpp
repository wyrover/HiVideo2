#include "stdafx.h"
#include "ImageDumper.h"

namespace e
{
	struct ThreadParams
	{
		CImageDumper* pDumper;
		CBitmap* pBG;
		CBitmap* pFG;
		CBitmap* pDiff;
		TCHAR    szFileName[MAX_PATH];
		ThreadParams(void)
		{
			pBG = NULL;
			pFG = NULL;
		}
		~ThreadParams(void)
		{
			SafeDelete(&pBG);
			SafeDelete(&pFG);
			SafeDelete(&pDiff);
		}
	};

	CImageDumper::CImageDumper(void)
	{
		m_bFinished = true;
		m_hThread = INVALID_HANDLE_VALUE;
	}

	CImageDumper::~CImageDumper(void)
	{
	}

	void CImageDumper::CompareImage(CBitmap* pDest, CBitmap* pSrc0, CBitmap* pSrc1)
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
				short dx = abs(B0 - B1) + abs(G0 - G1) + abs(R0 - R1);
				(*(short*)p2) = dx;
				p2[2] = abs(A0 - A1);

				p0 += nPixelSize;
				p1 += nPixelSize;
				p2 += nPixelSize;
			}
		}
	}

	void CImageDumper::SaveCompareInfo(const TCHAR* lpFileName, CBitmap* pDiff)
	{
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
				int nValue = (*(short*)p);
				nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "%03d ", nValue);
				p += nBitCount >> 3;
			}
			nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "\r\n");
		}

		nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset,
			"----------------------------------------------------------------------------------------------------------------------------\r\n");

		for (int y = 0; y < nHeight; y++)
		{
			uint8* p = pDiff->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int nValue = p[2];
				nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "%03d ", nValue);
				p += nBitCount >> 3;
			}
			nOffset += sprintf_s(pBuffer + nOffset, nBufferSize - nOffset, "\r\n");
		}

		do
		{
			HANDLE hFile = CreateFile(lpFileName
				, GENERIC_WRITE
				, 0
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_NORMAL
				, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				Log(_T("open file failed : %u, %s"), GetLastError(), lpFileName);
				break;
			}

			DWORD dwWroteBytes = 0;
			WriteFile(hFile, pBuffer, strlen(pBuffer), &dwWroteBytes, NULL);
			FlushFileBuffers(hFile);
			CloseHandle(hFile);
		} while (0);

		if (pBuffer) delete[] pBuffer;
	}

	void CImageDumper::Difference(const TCHAR* pFileName, CBitmap* pBG, CBitmap* pFG)
	{
		if (!m_bFinished) return;
		if (m_hThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hThread);
			m_hThread = INVALID_HANDLE_VALUE;
		}
		ThreadParams* pParams = new ThreadParams();
		pParams->pBG = new CBitmap(*pBG);
		pParams->pFG = new CBitmap(*pFG);
		pParams->pDiff = new CBitmap(*pBG);
		_tcscpy_s(pParams->szFileName, pFileName);

		m_hThread = CreateThread(NULL
			, 0
			, ThreadProxy
			, (LPVOID)pParams
			, 0
			, NULL);
	}

	DWORD CImageDumper::ThreadProc(LPVOID lpParam)
	{
		ThreadParams* pThreadInfo = static_cast<ThreadParams*>(lpParam);
		CBitmap* pBG = pThreadInfo->pBG;
		CBitmap* pFG = pThreadInfo->pFG;;
		CBitmap* pDiff = pThreadInfo->pDiff;

		CompareImage(pDiff, pBG, pFG);
		SaveCompareInfo(pThreadInfo->szFileName, pDiff);
		delete lpParam;
		m_bFinished = true;
		return 0;
	}

	DWORD CImageDumper::ThreadProxy(LPVOID lpParam)
	{
		ThreadParams* param = (ThreadParams*)lpParam;
		return param->pDumper->ThreadProc(lpParam);
	}
}