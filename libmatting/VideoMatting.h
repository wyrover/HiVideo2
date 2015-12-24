#pragma once

namespace e
{
#define STATE_RESET 0
#define STATE_INITIALIZE	0x00000001
#define STATE_SET_RBG	0x00000002
#define STATE_SET_VBG	0x00000004
#define STATE_MATTING	0x00000008

#define ENABLE_SPLITBLOCK

	enum ImageType{
		temp_image,
		real_bg_image,
		real_bg_gray,
		real_bg_mask,
		current_image,
		current_gray,
		current_mask,
		virtual_bg_image,
		_max_image_count
	};

	class CBitmap;
	class CImageFilter;
	class CVideoMatting
	{
	public:
		CVideoMatting(void);
		virtual ~CVideoMatting(void);
		bool InitializeImage(int nWidth, int nHeight, int nBitCount);
		void SetRealBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetThreshold(int nThreshold);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void OnSampleSave(const TCHAR* pFileName);
		void Reset(void);
	protected:
		CBitmap* GetBitmap(ImageType eType) const;
		bool IsReadyMatting(void) const;
		void Gray(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void Gray(void* pGray, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void PreprecessBackground(void);
		void CalcMask(void* pMask, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void CalcMatting(void* pData, void* pMask, int nWidth, int nHeight, int nBitCount);
		void Clean(void);
	protected:
		int m_nState;
		int m_nThreshold;
		int m_nBlockSize;
		int m_nNumberOfImage;
		CBitmap** m_pImages;
		CImageFilter* m_pFilter;
	};
}
