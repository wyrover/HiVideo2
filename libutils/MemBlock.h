#pragma once
namespace e
{
	class CMemBlock
	{
	public: 
		CMemBlock(void);
		CMemBlock(int nSize);
		CMemBlock(void* pData, int nSize);
		CMemBlock(const CMemBlock& other);
		virtual ~CMemBlock(void);
		CMemBlock& operator=(const CMemBlock& other);
	public:
		bool Create(int nSize);
		bool Create(void* pData, int nSize);
		int GetSize(void) const;
		void* GetData(void) const;
		bool SetData(void* pData, int nSize);
		void Destroy(void);
	public:
		int m_nSize;
		void* m_pData;
	};
}

