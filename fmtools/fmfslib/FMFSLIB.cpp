// FMFSLIB.cpp : DLL �A�v���P�[�V�����p�̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "FMFSLIB.h"

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


// ����̓G�N�X�|�[�g���ꂽ�ϐ��̗�ł��B
//FMFSLIB_API int nFMFSLIB=0;

// ����̓G�N�X�|�[�g���ꂽ�֐��̗�ł��B
//FMFSLIB_API int fnFMFSLIB(void)
//{
//	return 42;
//}

// ����̓G�N�X�|�[�g���ꂽ�N���X�̃R���X�g���N�^�ł��B
// �N���X�̒�`�ɂ��Ă� FMFSLIB.h ���Q�Ƃ��Ă��������B
//CFMFSLIB::CFMFSLIB()
//{ 
//	return; 
//}

