#include "stdafx.h"
#include "MainWindow.h"

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\"));

	CMainWindow* pMainFrame = new CMainWindow();
	if (pMainFrame == NULL) return 0;
	pMainFrame->Create(NULL, NULL, UI_WNDSTYLE_DIALOG, 0);
	pMainFrame->CenterWindow();
	pMainFrame->ShowWindow(true);
	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	return 0;
}