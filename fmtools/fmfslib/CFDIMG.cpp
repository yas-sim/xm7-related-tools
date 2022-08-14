#include "stdafx.h"
#include "CFDIMG.H"

#if VERBOSE==1
#include <stdio.h>
#endif

// 000318 WriteSector�֐���fWriteProtect�t���O���m�F����悤�ɕύX

// �w��̃g���b�N�ւ̃I�t�Z�b�g�����߂�
long CFDImg::GetTrackOffset( int C, int H ) {
	int TrackNo;
	TrackNo = C*2+H;
	return m_nTrackOffset[TrackNo];
}

// �w���CHR�����Z�N�^�ւ̃I�t�Z�b�g�����߂�
long CFDImg::GetSectorOffset( int C, int H, int R ) {
	long Offset = GetTrackOffset(C, H);
	if(Offset==0) return 0L;
	int sect_count = 0;
	CSectorImg sect;
	do {
		ReadSectorID(Offset, &sect);
		if(sect.m_nC==C && sect.m_nH==H && sect.m_nR==R) break;
		Offset += (sect.m_nSectorSize+16);		// 16 is a size of sector header
	} while(sect_count++ < sect.m_nNumberOfSectors);
	return Offset;
}

// �w��̃I�t�Z�b�g�ʒu����Z�N�^��ID����ǂݎ��
// �Z�N�^�f�[�^�̓ǂݏo���͍s��Ȃ�
bool CFDImg::ReadSectorID( long Offset, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	DWORD NOR;
	CSectorImg *scti = (CSectorImg*)sect;
	unsigned char reserve[5];
	SetFilePointer(m_hFDImage, Offset, NULL, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nC,				1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nH,				1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nR,				1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nN,				1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nNumberOfSectors,	2, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_fDensity,			1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_fDDM,				1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nStatus,			1, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)reserve,					5, &NOR, NULL) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nSectorSize,		2, &NOR, NULL) == 0) { SHOW_ERROR; }
	return true;
}

// �w���CHR�̃Z�N�^��ǂݏo��
// sect��m_pSectorData���w���̈�Ƀf�[�^���������ށB
// m_pSectorData�ɂ͗̈���m�ۂ��Ă����K�v����
FDC_STATUS	CFDImg::ReadSector( int C, int H, int R, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	long Offset;
	DWORD NOR;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;											// Sector not found
	ReadSectorID(Offset, sect);
//	if(sect->m_pSectorData!=NULL) delete[](sect->m_pSectorData);		// Release reserved memory area
//	sect->m_pSectorData = new unsigned char [sect->m_nSectorSize];	// Reserve memory area 
//	if(sect->m_pSectorData==NULL) return 2;							// Not enough memory
	if(ReadFile(m_hFDImage, (LPVOID)sect->m_pSectorData, sect->m_nSectorSize, &NOR, NULL) == 0) { SHOW_ERROR; }
	return 0;
}

// �w���CHR�̃Z�N�^�ɁAsect.m_pSectorData���w���̈�̃f�[�^����������
// �w��̃Z�N�^���Ȃ��ꍇ�͏������݂��s��Ȃ�(�V�K�ɃZ�N�^���������͂��Ȃ�)
// sect�̎����Ă���CHRN�͎g�p���Ȃ�(������CHR�̂ݗL��)
FDC_STATUS	CFDImg::WriteSector( int C, int H, int R, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	long Offset;
	DWORD NOW;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;		// Sector not found
	ReadSectorID(Offset, sect);	// SectorID��ǂݔ�΂��A�f�[�^���̓�����������
	if(m_fWriteProtect==0x10) return FDC_WRITE_PROTECTED;	// Write Protected!
	if(WriteFile(m_hFDImage, (LPCVOID)sect->m_pSectorData, sect->m_nSectorSize, &NOW, NULL) == 0) { SHOW_ERROR; }
	return 0;
}

// FD Image�t�@�C�����I�[�v������B���������ꍇ�A�����I�Ƀw�b�_�����ǂݍ���
bool CFDImg::OpenFDImage( unsigned char *pFileName )
{
	m_hFDImage = CreateFile( (LPCTSTR)pFileName, GENERIC_READ | GENERIC_WRITE, 0,
						NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if(m_hFDImage==INVALID_HANDLE_VALUE) {
		SHOW_ERROR;
		return false;
	}
	if(ReadHeader()==false) return false;
	return true;
}

// FD Image�t�@�C�����N���[�Y����
bool CFDImg::CloseFDImage( void ) {
	if(m_hFDImage!=INVALID_HANDLE_VALUE) {
		CloseHandle(m_hFDImage);
		m_hFDImage = INVALID_HANDLE_VALUE;
	}
	return true;
}

// FD Image�̃w�b�_����ǂݏo��
bool CFDImg::ReadHeader( void ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return false;
	DWORD NOR;		// Number Of Read
	SetFilePointer(m_hFDImage, 0, NULL, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)m_sDiskName,		17, &NOR, NULL) == 0) { SHOW_ERROR; }
	SetFilePointer(m_hFDImage, 17+9, NULL, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)&m_fWriteProtect,	1, &NOR, NULL)==0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&m_fDiskType,		1, &NOR, NULL)==0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&m_nDiskSize,		4, &NOR, NULL)==0) { SHOW_ERROR; }
	for(int i=0; i<=_CFDIMAGE_MAX_TRACK_; i++) {
		if(ReadFile(m_hFDImage, (LPVOID)&m_nTrackOffset[i],	4, &NOR, NULL)==0) { SHOW_ERROR; }
	}
	return true;
}

// �e�c�C���[�W�f�[�^�̃w�b�_�����R���\�[���ɕ\������
// fShowTrackOfset��true���w�肷��ƁA�S�g���b�N�̃I�t�Z�b�g�l���\������
void CFDImg::ShowFDInfo( bool fShowTrackOffset ) {
#if VERBOSE==1
	printf("Disk Name :'%s'\n", m_sDiskName);
	printf("Write Protect :");
	switch(m_fWriteProtect) {
	case 0x00:
		printf("Off\t");
		break;
	case 0x10:
		printf("On\t");
		break;
	default:
		printf("Unrecognize write protect flag value : 0x%02x\t", m_fWriteProtect);
		break;
	}
	printf("Disk Type :");
	switch(m_fDiskType) {
	case 0x00:
		printf("2D\t");
		break;
	case 0x10:
		printf("2DD\t");
		break;
	case 0x20:
		printf("2HD\t");
		break;
	default:
		printf("Unrecognize disk type value : 0x%02x\t", m_fDiskType);
	}
	printf("Disk Size :%ldByte(s)\n", m_nDiskSize);
	if(fShowTrackOffset==true) {
		printf("Track Offset:\n");
		for(int i=0; i<=_CFDIMAGE_MAX_TRACK_; i++) {
			printf("%03d : %04lx\t", i, m_nTrackOffset[i]);
		}
	}
#endif
}
