#ifndef __E_LIBUTILDEFS_H__
#define __E_LIBUTILDEFS_H__
namespace e
{
	// 不定长整形
	typedef char int8;
	typedef short int16;
	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned short ushort;
	typedef unsigned char uchar;
	//typedef unsigned char byte;

	// 定长整型
#if defined(_WIN64) || defined(__x86_64__)
#define E_64
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef long long intx;
	typedef unsigned long long uintx;
#else
#define E_32
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef int intx;
	typedef unsigned int uintx;
#endif

#ifndef WidthBytes
#define WidthBytes(bits) ((((bits)+31) & (~31)) / 8)
#endif

	template<class T> void SafeFree(T** p)
	{
		if (*p)
		{
			free(*p);
			*p = NULL;
		}
	}

	template<class T> void SafeDelete(T** p)
	{
		if (*p)
		{
			delete *p;
			*p = NULL;
		}
	}

	template<class T> void SafeDeleteArray(T** p)
	{
		if (*p)
		{
			delete[] *p;
			*p = NULL;
		}
	}

	template<class T> void SafeRelease(T** p)
	{
		if (*p)
		{
			(*p)->Release();
			*p = NULL;
		}
	}

}
#endif