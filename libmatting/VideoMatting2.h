#pragma once
namespace e
{
	enum ImageType2{
		background,		//32bit
		bg_block,			//32bit
		foreground,		//32bit
		fg_gray,			//8bit
		graph,				//8bit
		edge,				//8bit
		trimap,				//8bit
		alpha,				//8bit
		_max_count_
	};

	enum PreprocOption{
		preproc_undo = 0x00,
		preproc_smooth = 0x01,
		preproc_gray = 0x02,
		preproc_block = 0x04,
		preproc_noisc = 0x08
	};

	enum MattingMode{
		SimpleMatting,
		NormalMatting
	};

	class CDifferen;
	class CImageFilter;
	class CVideoMatting2
	{
	public:
		CVideoMatting2();
		virtual ~CVideoMatting2();
		bool Initialize(int nWidth, int nHeight, int nBitCount);
		void SetMattingMode(MattingMode eMode);
		void SetMattingThreshold(int nIndex, int nThreshold);
		void SetBackground(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetVirtualBGImage(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void RemoveNoiscEnable(bool bEnable);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void OnSampleSave(TCHAR* pFileName);
		void Reset(void);
	protected:
		CBitmap* GetBitmap(ImageType2 eType) const;
		void SetPreprocOption(PreprocOption eOption, bool bAdd);
		void PreprocessImage(CBitmap* pImage);
		void RemoveNoisc(CBitmap* pGraph);
		void CalcEdge(CBitmap* pEdge, CBitmap* pGraph);
		void CalcMerge(CBitmap* pGraph, CBitmap* pEdge);
		void CalcMerge(CBitmap* pTrimap, CBitmap* pGraph, CBitmap* pEdge);
		void MakeGraph(CBitmap* pGraph, CBitmap* pBG, CBitmap* pFG);
		void FillRect(int x, int y, int size, int c, BYTE* pData, int nBitCount, int nLineSize);
		void FillRound(int x, int y, int r, BYTE* pData, int nWidth, int nHeight, int nBitCount);
		void MakeAlpha(CBitmap* pAlpha, CBitmap* pTrimap, CBitmap* pBG, void* pFG);
		void MakeMatting(void* pData, CBitmap* pAlpha, CBitmap* pVG);
	protected:
		int m_nNumberOfBitmap;
		CBitmap** m_pBitmaps;

		int m_nBlockSize;
		int m_nMattingMode;
		int m_nPreprocOption;
		int m_nDistanceThreshold;
		int m_nNoiscThreshold;
		int m_nDifferenThreshold;

		CDifferen* m_pDiffer;
		CImageFilter* m_pFilter;
	};
}
