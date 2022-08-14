// FMFSLIB.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//
#define FMFSLIB_EXPORTS

#include "stdafx.h"
#include "FMFSLIB.h"
#if 0
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

// これはエクスポートされた変数の例です。
//FMFSLIB_API int nFMFSLIB=0;

// これはエクスポートされた関数の例です。
//FMFSLIB_API int fnFMFSLIB(void)
//{
//	return 42;
//}

// これはエクスポートされたクラスのコンストラクタです。
// クラスの定義については FMFSLIB.h を参照してください。
//CFMFSLIB::CFMFSLIB()
//{ 
//	return; 
//}

