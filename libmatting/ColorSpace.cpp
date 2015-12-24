#include "stdafx.h"
#include "ColorSpace.h"

namespace e
{
	void RGB2HSV(uint8 &r, uint8 &g, uint8 &b, float* hval, float* sval, float* vval)
	{
		float r_ = r / 256.0;
		float g_ = g / 256.0;
		float b_ = b / 256.0;

		float min_rgb = min(r_, min(g_, b_));
		float max_rgb = max(r_, max(g_, b_));
		float V = max_rgb;
		float H = 0.0;
		float S = 0.0;

		float delta = max_rgb - min_rgb;

		if ((delta > 0.0) && (max_rgb > 0.0))
		{
			S = delta / max_rgb;
			if (max_rgb == r_)
				H = (g_ - b_) / delta;
			else if (max_rgb == g_)
				H = 2 + (b_ - r_) / delta;
			else
				H = 4 + (r_ - g_) / delta;
		}

		H /= 6;

		/*float V = max(r_,max(g_,b_));
		float S = 0.0;
		if (V != 0.0) S = (V - min(r_, min(g_,b_)))/V;
		float H = 0.0;
		if (S != 0.0)
		{
		if (V == r_)
		H = ((g_ - b_)/6.0)/S;
		else if (V == g_)
		H = 1.0/2.0 + ((b_-r_)/6.0)/S;
		else
		H = 2.0/3.0 + ((r_-g_)/6.0)/S;
		}*/

		if (H<0.0) H += 1.0;

		*hval = H;
		*sval = S;
		*vval = V;
	}

	void RGB2LAB(uint8 &r, uint8 &g, uint8 &b, float* lval, float* aval, float* bval)
	{
		float xVal = 0.412453f * r + 0.357580f * g + 0.180423f * b;
		float yVal = 0.212671f * r + 0.715160f * g + 0.072169f * b;
		float zVal = 0.019334f * r + 0.119193f * g + 0.950227f * b;

		xVal /= (255.0f * 0.950456f);
		yVal /= 255.0f;
		zVal /= (255.0f * 1.088754f);

		float fY, fZ, fX;
		float lVal, aVal, bVal;
		float T = 0.008856f;

		bool XT = (xVal > T);
		bool YT = (yVal > T);
		bool ZT = (zVal > T);

		fX = XT * pow(xVal, 1.0 / 3.0) + (!XT) * (7.787 * xVal + 16.0 / 116.0);

		// Compute L
		float Y3 = pow(yVal, 1.0 / 3.0);
		fY = YT*Y3 + (!YT)*(7.787*yVal + 16.0 / 116.0);
		lVal = YT * (116 * Y3 - 16.0) + (!YT)*(903.3*yVal);

		fZ = ZT*pow(zVal, 1.0 / 3.0) + (!ZT)*(7.787*zVal + 16.0 / 116.0);

		// Compute a and b
		aVal = 500 * (fX - fY);
		bVal = 200 * (fY - fZ);

		*lval = lVal;
		*aval = aVal;
		*bval = bVal;
	}
}
