#include "stdafx.h"
#include "MemBlock.h"

namespace e
{
	CMemBlock::CMemBlock(void)
	{
		m_nSize = 0;
		m_pData = NULL;
	}

	CMemBlock::CMemBlock(int nSize)
	{
		m_nSize = 0;
		m_pData = NULL;
		bool bRet = Create(nSize);
		assert(bRet);
	}

	CMemBlock::CMemBlock(void* pData, int nSize)
	{
		m_nSize = 0;
		m_pData = NULL;
		bool bRet = Create(pData, nSize);
		assert(bRet);
	}

	CMemBlock::CMemBlock(const CMemBlock& other)
	{
		m_nSize = 0;
		m_pData = NULL;
		bool bRet = Create(other.GetData(), other.GetSize());
		assert(bRet);
	}

	CMemBlock::~CMemBlock(void)
	{
		Destroy();
	}

	int CMemBlock::GetSize(void) const
	{
		return m_nSize;
	}

	void*CMemBlock::GetData(void) const
	{
		return m_pData;
	}

	bool CMemBlock::Create(int nSize)
	{
		if (nSize <= 0) return false;
		if (m_nSize != nSize)
		{
			m_pData = realloc(m_pData, nSize);
			assert(m_pData);
			if (m_pData == NULL)return false;
			m_nSize = nSize;
		}
		return true;
	}

	bool CMemBlock::Create(void* pData, int nSize)
	{
		if (m_nSize != nSize)
		{
			m_pData = realloc(m_pData, nSize);
			assert(m_pData);
			if (m_pData == NULL)return false;
			m_nSize = nSize;
		}
		memcpy(m_pData, pData, nSize);
		return true;
	}

	bool CMemBlock::SetData(void* pData, int nSize)
	{
		return Create(pData, nSize);
	}

	CMemBlock& CMemBlock::operator=(const CMemBlock& other)
	{
		if (this != &other)
		{
			if (!Create(other.GetData(), other.GetSize()))
			{
				assert(0);
			}
		}
		return *this;
	}

	void CMemBlock::Destroy(void)
	{
		if (m_pData != NULL)
		{
			free(m_pData);
			m_pData = NULL;
		}
	}
}