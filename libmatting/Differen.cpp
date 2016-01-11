#include "stdafx.h"
#include "Differen.h"

namespace e
{
	const BYTE g_sub_table[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
		128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
		176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
		192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
		208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
		224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
		240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255
	};

	inline int square(int x)
	{
		return x * x;
	}

	inline int rgb_diff(BYTE* p0, BYTE* p1)
	{
		return abs(p0[0] - p1[0]) + abs(p0[1] - p1[1]) + abs(p0[2] - p1[2]);
	}

	inline int rgb_diff(int r0, int b0, int g0, int r1, int b1, int g1)
	{
		return abs(r0 - r1) + abs(b0 - b1) + abs(g0 - g1);
	}

	inline int rgb_dist(BYTE* p0, BYTE* p1)
	{
		return (int)sqrt(square(p0[0] - p1[0]) + square(p0[1] - p1[1]) + square(p0[2] - p1[2]));
	}

	inline int rgb_dist(int r0, int b0, int g0, int r1, int b1, int g1)
	{
		return (int)sqrt(square(r0 - r1) + square(b0 - b1) + square(g0 - g1));
	}

	CDifferen::CDifferen()
	{
		m_nThreshold = 30;
	}

	CDifferen::~CDifferen()
	{
	}

	void CDifferen::SetThreshold(int nThreshod)
	{
		m_nThreshold = nThreshod;
	}

	void CDifferen::Differen(CBitmap* pAlpha, CBitmap* pBG, CBitmap* pFG)
	{
		int nWidth = pBG->Width();
		int nHeight = pBG->Height();
		int nBitCount = pBG->BitCount();
		int nLineBytes = pBG->LineSize();

		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = pBG->GetBits(0, y);
			BYTE* pF = pFG->GetBits(0, y);
			BYTE* pA = pAlpha->GetBits(0, y);
			for (int x = 0; x < nWidth; x++)
			{
				int dx = max(0, min(rgb_dist(pB[0], pB[1], pB[2], pF[0], pF[1], pF[2]), 255));
				*pA++ = dx > m_nThreshold ? dx : 0;
				pB += nBitCount >> 3;
				pF += nBitCount >> 3;
			}
		}
	}

	void CDifferen::Differen(void* pData, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nWidth*nBitCount);
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = (BYTE*)pBG + y * nLineBytes;
			BYTE* pF = (BYTE*)pFG + y * nLineBytes;
			BYTE* pA = (BYTE*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x++)
			{
				int dx = max(0, min(rgb_dist(pB[0], pB[1], pB[2], pF[0], pF[1], pF[2]), 255));
				pA[0] = pA[1] = pA[2] = dx <= m_nThreshold ? 0 : dx;
				pA += nBitCount >> 3;
				pB += nBitCount >> 3;
				pF += nBitCount >> 3;
			}
		}
	}

	void CDifferen::Sub(void* pData, void* pBG, void* pFG, int nWidth, int nHeight, int nBitCount)
	{
		int nLineBytes = WidthBytes(nWidth*nBitCount);
		for (int y = 0; y < nHeight; y++)
		{
			BYTE* pB = (BYTE*)pBG + y * nLineBytes;
			BYTE* pF = (BYTE*)pFG + y * nLineBytes;
			BYTE* pA = (BYTE*)pData + y * nLineBytes;
			for (int x = 0; x < nWidth; x++)
			{
// 				pA[0] = g_sub_table[(pB[0] - pF[0])+256];
// 				pA[1] = g_sub_table[(pB[1] - pF[1])+256];
// 				pA[2] = g_sub_table[(pB[2] - pF[2])+256];

				pA[0] = abs(pB[0] - pF[0]);
				pA[1] = abs(pB[1] - pF[1]);
				pA[2] = abs(pB[2] - pF[2]);

				pA += nBitCount >> 3;
				pB += nBitCount >> 3;
				pF += nBitCount >> 3;
			}
		}
	}

	inline int sample_weight(int x, int y, BYTE* p0, int w, int h, int bpp, int line, int t)
	{
		BYTE* p1 = (y > 0) ? p0 - line : p0;
		BYTE* p2 = (y < h - 1) ? p0 + line : p0;
		BYTE* p3 = (x > 0) ? p0 - bpp : p0;
		BYTE* p4 = (x < w - 1) ? p0 + bpp : p0;

		BYTE* p5 = (x > 0) ? p1 - bpp : p1;
		BYTE* p6 = (x < w - 1) ? p1 + bpp : p1;
		BYTE* p7 = (x > 0) ? p2 - bpp : p2;
		BYTE* p8 = (x < w - 1) ? p2 + bpp : p2;

		int dist = 0, sum = 0, count = 0;
		dist = rgb_diff(p0, p1);
		if (dist >= t) { sum += dist; count++; }
		dist = rgb_diff(p0, p2);
		if (dist >= t) { sum += dist; count++; }
		dist = rgb_diff(p0, p3);
		if (dist >= t) { sum += dist; count++; }
		dist = rgb_diff(p0, p4);
		if (dist >= t) { sum += dist; count++; }

// 		dist = rgb_diff(p0, p5);
// 		if (dist >= t) { sum += dist; count++; }
// 		dist = rgb_diff(p0, p6);
// 		if (dist >= t) { sum += dist; count++; }
// 		dist = rgb_diff(p0, p7);
// 		if (dist >= t) { sum += dist; count++; }
// 		dist = rgb_diff(p0, p8);
// 		if (dist >= t) { sum += dist; count++; }

		return count > 0 ? sum / count : 0;
	}

	void CDifferen::SampleWeight(void* pData, int nWidth, int nHeight, int nBitCount, int nThreshold)
	{
		int nPixelSize = nBitCount >> 3;
		int nLineSize = WidthBytes(nWidth*nBitCount);

		for (int y = 0; y < nHeight; y++)
		{
			BYTE* p = (BYTE*)pData + y * nLineSize;
			for (int x = 0; x < nWidth; x++)
			{
				int dx = sample_weight(x, y, p, nWidth, nHeight, nPixelSize, nLineSize, nThreshold);
				p[0] = p[1] = p[2] = max(0, min(dx/3, 255));
				p += nPixelSize;
			}
		}
	}
}
