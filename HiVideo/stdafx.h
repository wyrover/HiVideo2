// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
//#include <windows.h>
#include <strmif.h>
#include <uuids.h>

// TODO:  在此处引用程序需要的其他头文件
#include "libutils.h"
#include "streams.h"
#include "libeffect.h"
#include "libmatting.h"
#include "libcamera.h"
#include "UIlib.h"
using namespace e;
using namespace DuiLib;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#pragma comment(lib, "libstreamd.lib")
#pragma comment(lib, "libeffectd.lib")
#pragma comment(lib, "libmattingd.lib")
#pragma comment(lib, "libcamerad.lib")
#pragma comment(lib, "libduid.lib")
#else
#pragma comment(lib, "libutils.lib")
#pragma comment(lib, "libstream.lib")
#pragma comment(lib, "libeffect.lib")
#pragma comment(lib, "libmatting.lib")
#pragma comment(lib, "libcamera.lib")
#pragma comment(lib, "libdui.lib")
#endif
