// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	// 1
	case DLL_THREAD_ATTACH:		// 2
	case DLL_THREAD_DETACH:		// 3
	case DLL_PROCESS_DETACH:	// 0
		break;
	}
	char buf[100];
	wsprintfA(buf, "DllMain hModule=%p, call=%lx, Reserved=%p", hModule, ul_reason_for_call, lpReserved);
	OutputDebugStringA(buf);
	return TRUE;
}

