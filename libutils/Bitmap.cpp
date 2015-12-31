#include "stdafx.h"
#include "Bitmap.h"
#include "Log.h"

namespace e
{
#ifndef  WidthBytes
#define WidthBytes(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#endif // !WIDTHBYTES

#ifndef CheckPointer
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}
#endif

#ifndef BI_RGB
#	define BI_RGB 0
#endif

#pragma pack(push, 1)
	typedef struct _RGBQUAD
	{
		BYTE red;
		BYTE green;
		BYTE blue;
		BYTE reserved;
	}RGBQUAD;

	typedef struct _BitmapHeader{
		WORD bfType;
		DWORD bfSize;
		WORD bfReserved1;
		WORD bfReserved2;
		DWORD bfOffBits;
		DWORD biSize;
		DWORD biWidth;
		DWORD biHeight;
		WORD biPlanes;
		WORD biBitCount;
		DWORD biCompression;
		DWORD biSizeImage;
		DWORD biXPelsPerMeter;
		DWORD biYPelsPerMeter;
		DWORD biClrUsed;
		DWORD biClrImportant;
	}BITMAPHEADER;
#pragma pack(pop)

	inline bool Read(void* pBuffer, DWORD dwSize, HANDLE hFile)
	{
		DWORD dwRead = 0;
		return ReadFile(hFile, pBuffer, dwSize, &dwRead, NULL) && (dwSize == dwRead);
	}

	inline bool Write(void* pBuffer, DWORD dwSize, HANDLE hFile)
	{
		DWORD dwWrite = 0;
		return ::WriteFile(hFile, pBuffer, dwSize, &dwWrite, NULL) && (dwSize == dwWrite);
	}

	inline bool Skip(DWORD dwSkip, DWORD dwMoveFlag, HANDLE hFile)
	{
		return SetFilePointer(hFile, dwSkip, NULL, dwMoveFlag) != INVALID_SET_FILE_POINTER;
	}

	inline void memswp(BYTE* dst, BYTE* src, int size)
	{
		BYTE tmp;
		for (int i = 0; i < size; i++)
		{
			tmp = *src;
			*src = *dst;
			*dst = tmp;
			src++;
			dst++;
		}
	}

	CBitmap::CBitmap(void)
	{
		Reset();
	}

	CBitmap::CBitmap(const CBitmap& other)
	{
		Reset();
		BOOL bRet = Create(other.m_nWidth
			, other.m_nHeight
			, other.m_nBitCount
			, other.m_pBits);
		assert(bRet);
	}

	CBitmap::CBitmap(LPCTSTR lpFileName)
	{
		Reset();
		BOOL bRet = Load(lpFileName);
		assert(bRet);
	}

	CBitmap::CBitmap(int nWidth, int nHeight, int nBitCount, LPVOID pBits /* = NULL */, bool bAlloc /* = true */)
	{
		Reset();

		if (bAlloc)
		{
			BOOL bRet = Create(nWidth, nHeight, nBitCount, pBits);
			assert(bRet);
		}
		else if (pBits)
		{
			m_pBits = (BYTE*)pBits;
			m_nWidth = nWidth;
			m_nHeight = nHeight;
			m_nBitCount = nBitCount;
			m_nLineSize = WidthBytes(nWidth*nBitCount);
		}
	}

	void CBitmap::Reset(void)
	{
		m_pBits = NULL;
		m_nSize = 0;
		m_nWidth = 0;
		m_nHeight = 0;
		m_nBitCount = 0;
		m_nLineSize = 0;
	}

	CBitmap::~CBitmap(void)
	{
		Destroy();
	}

	int CBitmap::Width(void) const
	{
		return m_nWidth;
	}

	int CBitmap::Height(void) const
	{
		return m_nHeight;
	}

	int CBitmap::BitCount(void) const
	{
		return m_nBitCount;
	}

	int CBitmap::PixelSize(void) const
	{
		return m_nBitCount >> 3;
	}

	int CBitmap::LineSize(void) const
	{
		return m_nLineSize;
	}

	int CBitmap::Size(void) const
	{
		return m_nSize;
	}

	BYTE* CBitmap::GetBits(void) const
	{
		return m_pBits;
	}

	BYTE* CBitmap::GetBits(int x, int y) const
	{
		assert(x >= 0 && x < m_nWidth);
		assert(y >= 0 && y < m_nHeight);
		assert(m_pBits);
		return m_pBits + (y * m_nLineSize) + x * (m_nBitCount >> 3);
	}

	BOOL CBitmap::Load(LPCTSTR lpFileName)
	{
		CheckPointer(lpFileName, FALSE);
		if (!CBitmap::Load(lpFileName
			, (LPDWORD)&m_nWidth
			, (LPDWORD)&m_nHeight
			, (LPDWORD)&m_nBitCount
			, (LPVOID*)&m_pBits
			, (LPDWORD)&m_nSize))
		{
			Destroy();
			return FALSE;
		}

		m_nLineSize = WidthBytes(m_nWidth*m_nBitCount);
		return TRUE;
	}

	BOOL CBitmap::Create(int nWidth, int nHeight, int nBitCount, LPVOID lpBits /* = NULL */, bool bSetZero /* = false */)
	{
		assert(nWidth >= 0 && nHeight >= 0 && nBitCount > 0);
		if (nWidth < 0 || nHeight < 0||nBitCount<=0) return FALSE;
		int nNewSize = WidthBytes(nWidth * nBitCount) * nHeight;
		if (nNewSize != m_nSize)
		{
			m_pBits = (BYTE*)realloc(m_pBits, nNewSize);
			if (m_pBits == NULL) return FALSE;
			m_nSize = nNewSize;
		}

		if (lpBits != NULL)
		{
			memcpy(m_pBits, lpBits, nNewSize);
		}
		else if (bSetZero)
		{
			memset(m_pBits, 0, nNewSize);
		}

		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nBitCount = nBitCount;
		m_nLineSize = WidthBytes(nWidth*nBitCount);
		return TRUE;
	}

	BOOL CBitmap::Store(void* pData, int nWidth, int nHeight, int nBitCount)
	{
		return Create(nWidth, nHeight, nBitCount, pData);
	}

	BOOL CBitmap::Store(CBitmap* pSrc)
	{
		assert(pSrc);
		if (pSrc == NULL) return false;
		return Create(pSrc->Width(), pSrc->Height(), pSrc->BitCount(), pSrc->GetBits());
	}

	BOOL CBitmap::Save(LPCTSTR lpFileName)
	{
		return CBitmap::Save(lpFileName, m_nWidth, m_nHeight, m_nBitCount, m_pBits);
	}

	const CBitmap& CBitmap::operator=(const CBitmap& other)
	{
		Create(other.m_nWidth, other.m_nHeight, other.m_nBitCount, other.m_pBits);
		return *this;
	}

	void CBitmap::Clear(void)
	{
		if (m_pBits){
			memset(m_pBits, 0, m_nSize);
		}
	}

	BOOL CBitmap::Load(LPCTSTR lpFileName
		, LPDWORD pWidth
		, LPDWORD pHeight
		, LPDWORD pBitCount
		, LPVOID* ppBits
		, LPDWORD pSize)
	{
		HANDLE hFile = NULL;
		BOOL bResult = FALSE;
		do
		{
			hFile = CreateFile(lpFileName
				, GENERIC_READ
				, 0
				, NULL
				, OPEN_ALWAYS
				, FILE_ATTRIBUTE_NORMAL
				, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				Log(_T("open file failed : %u, %s"), GetLastError(), lpFileName);
				break;
			}

			DWORD dwFileSize = GetFileSize(hFile, NULL);
			if (dwFileSize <= 54) break;

			BITMAPHEADER header = { 0 };
			//read bitmap file header
			if (!Read(&header.bfType, sizeof(header.bfType), hFile)) break;
			if (header.bfType != 0x4D42) break;
			if (!Read(&header.bfSize, sizeof(header.bfSize), hFile)) break;
			if (!Read(&header.bfReserved1, sizeof(header.bfReserved1), hFile)) break;
			if (!Read(&header.bfReserved2, sizeof(header.bfReserved2), hFile)) break;
			if (!Read(&header.bfOffBits, sizeof(header.bfOffBits), hFile)) break;
			//read bitmap info header
			if (!Read(&header.biSize, sizeof(header.biSize), hFile)) break;
			if (!Read(&header.biWidth, sizeof(header.biWidth), hFile)) break;
			if (!Read(&header.biHeight, sizeof(header.biHeight), hFile)) break;
			if (!Read(&header.biPlanes, sizeof(header.biPlanes), hFile)) break;
			if (!Read(&header.biBitCount, sizeof(header.biBitCount), hFile)) break;
			if (!Read(&header.biCompression, sizeof(header.biCompression), hFile))	break;
			if (!Read(&header.biSizeImage, sizeof(header.biSizeImage), hFile)) break;
			if (!Read(&header.biXPelsPerMeter, sizeof(header.biXPelsPerMeter), hFile)) break;
			if (!Read(&header.biYPelsPerMeter, sizeof(header.biYPelsPerMeter), hFile))	break;
			if (!Read(&header.biClrUsed, sizeof(header.biClrUsed), hFile)) break;
			if (!Read(&header.biClrImportant, sizeof(header.biClrImportant), hFile)) break;
			if (header.biBitCount != 8 && header.biBitCount != 24 && header.biBitCount != 32) break;

			int nLineSize = WidthBytes(header.biWidth * header.biBitCount);
			int nNewSize = nLineSize * header.biHeight;
			*ppBits = (BYTE*)malloc( nNewSize);
			if (*ppBits == NULL)break; 

			if (!Skip(header.bfOffBits, FILE_BEGIN,hFile)) break;
			if (!Read(*ppBits, nNewSize, hFile)) break;

			*pWidth = (int)header.biWidth;
			*pHeight = (int)header.biHeight;
			*pBitCount = (int)header.biBitCount;
			*pSize = nNewSize;

			BYTE* p0 = (BYTE*)*ppBits;
			BYTE* p1 = (BYTE*)*ppBits + nNewSize-nLineSize;
			for (DWORD i = 0; i < header.biHeight/2; i++)
			{
				memswp(p0, p1, nLineSize);
				p0 += nLineSize;
				p1 -= nLineSize;
			}
			bResult = TRUE;
		} while (0);

		if (hFile) ::CloseHandle(hFile);
		return bResult;
	}

	BOOL CBitmap::Save(LPCTSTR lpFileName, DWORD dwWidth, DWORD dwHeight, DWORD dwBitCount, LPVOID lpBits)
	{
		HANDLE hFile = NULL;
		BOOL bResult = FALSE;
		do
		{
			hFile = CreateFile(lpFileName
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

			DWORD dwLineBytes = WidthBytes(dwWidth * dwBitCount);
			DWORD dwImageSize = dwLineBytes * dwHeight;
			DWORD dwPaletteSize = (1 << dwBitCount) * sizeof(RGBQUAD);

			BITMAPHEADER header = { 0 };
			header.bfType = 0x4D42;
			header.bfSize = 0;
			header.bfReserved1 = 0;
			header.bfReserved2 = 0;
			header.bfOffBits = 0;

			if (dwBitCount == 8)
			{
				header.bfOffBits = 54 + dwPaletteSize;
				header.bfSize = 54 + dwPaletteSize + dwImageSize;
			}
			else if (dwBitCount == 24 || dwBitCount == 32)
			{
				header.bfOffBits = 54;
				header.bfSize = 54 + dwImageSize;
			}

			header.biSize = 40;
			header.biWidth = dwWidth;
			header.biHeight = dwHeight;
			header.biPlanes = 1;
			header.biBitCount = (WORD)dwBitCount;
			header.biCompression = BI_RGB;
			header.biSizeImage = dwImageSize;
			header.biXPelsPerMeter = 3780;
			header.biYPelsPerMeter = 3780;
			header.biClrUsed = 0;
			header.biClrImportant = 0;

			if (!Write(&header, sizeof(header), hFile))
			{
				Log(_T("write file failed : %u, %s"), GetLastError(), lpFileName);
				break;
			}

			if (dwBitCount == 8)
			{
				RGBQUAD* pRGBQuad = new RGBQUAD[1 << dwBitCount];
				assert(pRGBQuad != 0);
				if (pRGBQuad == NULL) break;

				for (int i = 0; i < (1 << dwBitCount); i++)
				{
					pRGBQuad[i].red = i;
					pRGBQuad[i].green = i;
					pRGBQuad[i].blue = i;
				}

				if (!Write(pRGBQuad, sizeof(RGBQUAD) * (1 << dwBitCount), hFile))
				{
					Log(_T("write file failed : %u, %s"), GetLastError(), lpFileName);
					delete[] pRGBQuad;break;
				}

				delete[] pRGBQuad;
			}
			//需要将文件中的数据上下倒转过来
			BYTE* p = (BYTE*)lpBits + (dwHeight - 1) * dwLineBytes;
			for (DWORD i = 0; i < dwHeight; i++)
			{
				if (!Write(p, dwLineBytes, hFile))
				{
					Log(_T("write file failed : %u, %s"), GetLastError(), lpFileName);
					goto _error;
				}
				p -= dwLineBytes;
			}
			bResult = TRUE;
		} while (0);
_error:
		if (hFile) ::CloseHandle(hFile);
		return bResult;
	}

	void CBitmap::Destroy(void)
	{
		SafeFree(&m_pBits);
	}
}