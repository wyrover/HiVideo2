#pragma once
#include "VideoDevice.h"
namespace e
{
	class CMainWindow
		: public IVideoDevice
		, public IVideoFormat
		, public IVideoSample
		, public INotifyUI
		, public CWindowWnd
	{
	public:
		CMainWindow(void);
		virtual ~CMainWindow(void);
		LPCTSTR GetWindowClassName(void) const { return _T("UIMainFrame"); };
		UINT GetClassStyle(void) const { return UI_CLASSSTYLE_DIALOG; };
		void OnFinalMessage(HWND /*hWnd*/) { delete this; };
		void Notify(TNotifyUI& msg);
		//video capture
		void OnAddDevice(const TCHAR* pszDeviceName) override;
		void OnAddFormat(const TCHAR* pszFormat)override;
		void OnAddSample(void* pData, int nSize, int nWidth, int nHeight, int nBitCount) override;
	protected:
		void Init(void);
		void OnMinimize(void);
		void OnMaximize(void);
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		bool InitVideo(void);
		bool SwitchDevice(void);
		bool SwitchFormat(void);
		void OnVideoStart(void);
		void OnVideoStop(void);
		void OnVideoMatting(void);
		void OnVideoType(void);
		void OnVideoSave(void);
		void OnVideoDistance(void);
		void OnGraphNoise(void);

	protected:
		CPaintManagerUI m_pm;
		CVideoDevice* m_pVideoDevice;
		CVideoMatting* m_pVideoMatting;
		CImageEffect* m_pVideoEffect;

		//capture background
		enum {WaitCaptureBG, StartCaptureBG, MattingVideo};
		DWORD m_dwState;
		DWORD m_dwStartTime;
	};
}
