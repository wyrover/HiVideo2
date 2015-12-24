#pragma once
namespace e
{
	void RGB2HSV(uint8 &r, uint8 &g, uint8 &b, float* hval, float* sval, float* vval);
	void RGB2LAB(uint8 &r, uint8 &g, uint8 &b, float* lval, float* aval, float* bval);
	void HSV2RGB(float &h, float &s, float &v, uint8* rval, uint8* gval, uint8* bval);
	void LAB2RBG(float &l, float &a, float &b, uint8* rval, uint8* gval, uint8* bval);
}
