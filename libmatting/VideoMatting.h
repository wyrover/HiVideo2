#pragma once

namespace e
{
#define STATE_RESET 0
#define STATE_INITIALIZE	0x00000001
#define STATE_SET_RBG	0x00000002
#define STATE_SET_VBG	0x00000004
#define STATE_MATTING	0x00000008

#define OPTION_GRAY				0x00000001
#define OPTION_SPLITBLOCK	0x00000002
#define OPTION_SMOOTH			0x00000004

//#define ENABLE_SPLITBLOCK

	enum ImageType{
		temp_image,
		real_bg_image,
		current_image,
		current_trimap,
		current_alpha,
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
		void SetMattingThreshold(int nThreshold);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void OnSampleSave(const TCHAR* pFileName);
		void Reset(void);
	protected:
		CBitmap* GetBitmap(ImageType eType) const;
		void SetOption(int nOption, bool bAdd);
		bool IsReadyMatting(void) const;
		bool StoreImage(ImageType eType, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void PreprocessImage(CBitmap* pImage);
		void PreprocessBackground(void);
		void PreprocessForeground(void);
		void CompareImage(CBitmap* pDest, CBitmap* pSrc0, CBitmap* pSrc1);
		void CalcTrimap(void* pTrimap, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void CalcAlpha(void* pAlpha, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void CalcMatting(void* pData, void* pAlpha, int nWidth, int nHeight, int nBitCount);
		void Clean(void);
	protected:
		int m_nState;
		int m_nOptions;
		int m_nThreshold;
		int m_nBlockSize;
		int m_nNumberOfImage;
		CBitmap** m_pImages;
		CImageFilter* m_pFilter;
	};
}
