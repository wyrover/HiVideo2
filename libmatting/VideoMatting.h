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
#define OPTION_GRAPHNOISE 0x00000008


	enum ImageType{
		real_bg_image,
		real_bg_graph,
		current_image,
		current_graph,
		current_trimap,
		current_alpha,
		_max_image_count
	};

	class CBitmap;
	class CImageFilter;
	class CImageDumper;
	class CVideoMatting
	{
	public:
		CVideoMatting(void);
		virtual ~CVideoMatting(void);
		bool InitializeImage(int nWidth, int nHeight, int nBitCount);
		void SetRealBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetMattingThreshold(int nIndex, int nThreshold);
		void SetGraphNoiseEnable(bool bEnable);
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
		void GraphProc(void);
		void CalcTrimap(void* pTrimap, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void CalcAlpha(void* pAlpha, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount);
		void CalcMatting(void* pData, void* pAlpha, int nWidth, int nHeight, int nBitCount);
		void Clean(void);
	protected:
		int m_nState;
		int m_nOptions;
		int m_nThreshold[3];
		int m_nBlockSize;
		int m_nNumberOfImage;
		CBitmap** m_pImages;
		CImageFilter* m_pFilter;
		CImageDumper* m_pDumper;
	};
}
