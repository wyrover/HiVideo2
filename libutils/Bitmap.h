#pragma once

namespace e
{
	class CBitmap
	{
	public:
		CBitmap(void);
		CBitmap(const CBitmap& other);
		CBitmap(LPCTSTR lpFileName);
		CBitmap(int nWidth
			, int nHeight
			, int nBitCount
			, LPVOID pBits = NULL
			, bool bAlloc = true);
		virtual ~CBitmap(void);
	public:
		int Width(void) const;
		int Height(void) const;
		int BitCount(void) const;
		int PixelSize(void) const;
		int LineSize(void) const;
		int Size(void) const;
		BYTE* GetBits(void) const;
		BYTE* GetBits(int x, int y) const;
		//create bitmap
		HRESULT Create(int nWidth
			, int nHeight
			, int nBitCount
			, LPVOID lpBits = NULL
			, bool bSetZero = false);
		//destroy bitmap
		HRESULT Destroy(void);
		//load from file
		HRESULT Load(LPCTSTR lpFileName);
		//write to file
		HRESULT Save(LPCTSTR lpFileName);

		static BOOL Load(LPCTSTR lpFileName
			, LPDWORD pWidth
			, LPDWORD pHeight
			, LPDWORD pBitCount
			, LPVOID* ppBits
			, LPDWORD pSize);

		static BOOL Save(LPCTSTR lpFileName
			, DWORD dwWidth
			, DWORD dwHeight
			, DWORD dwBitCount
			, LPVOID lpBits);

		const CBitmap & operator=(const CBitmap& other);
	protected:
		void Reset(void);
	protected:
		 BYTE* m_pBits;
		UINT m_nSize;
		int m_nWidth;
		int m_nHeight;
		int m_nBitCount;
		int m_nLineSize;
	};
}