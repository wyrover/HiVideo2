#pragma once

namespace e
{
	enum BlurMode{ 
		Simple,
		Normal,
		SimpleSSE, 
		NormalSSE
	};

	void FastBlur(void* dst
		, void* src
		, int width
		, int height
		, int bitCount
		, float sigma
		, BlurMode mode);
}
