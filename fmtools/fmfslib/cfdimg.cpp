#define FMFSLIB_EXPORTS

#include "cfdimg.h"

#if VERBOSE==1
#include <stdio.h>
#endif

// 000318 WriteSector関数でfWriteProtectフラグを確認するように変更

// 指定のトラックへのオフセットを求める
long CFDImg::GetTrackOffset( int C, int H ) {
	int TrackNo;
	TrackNo = C*2+H;
	return m_nTrackOffset[TrackNo];
}

// 指定のCHRを持つセクタへのオフセットを求める
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

// 指定のオフセット位置からセクタのID情報を読み取る
// セクタデータの読み出しは行わない
bool CFDImg::ReadSectorID( long Offset, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	DWORD NOR;
	CSectorImg *scti = (CSectorImg*)sect;
	unsigned char reserve[5];
	SetFilePointer(m_hFDImage, Offset, nullptr, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nC,				1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nH,				1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nR,				1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nN,				1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nNumberOfSectors,	2, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_fDensity,			1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_fDDM,				1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nStatus,			1, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)reserve,					5, &NOR, nullptr) == 0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&scti->m_nSectorSize,		2, &NOR, nullptr) == 0) { SHOW_ERROR; }
	return true;
}

// 指定のCHRのセクタを読み出す
// sectのm_pSectorDataが指す領域にデータを書き込む。
// m_pSectorDataには領域を確保しておく必要あり
FDC_STATUS	CFDImg::ReadSector( int C, int H, int R, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	long Offset;
	DWORD NOR;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;											// Sector not found
	ReadSectorID(Offset, sect);
//	if(sect->m_pSectorData!=nullptr) delete[](sect->m_pSectorData);		// Release reserved memory area
//	sect->m_pSectorData = new unsigned char [sect->m_nSectorSize];	// Reserve memory area 
//	if(sect->m_pSectorData==nullptr) return 2;							// Not enough memory
	if(ReadFile(m_hFDImage, (LPVOID)sect->m_pSectorData, sect->m_nSectorSize, &NOR, nullptr) == 0) { SHOW_ERROR; }
	return 0;
}

// 指定のCHRのセクタに、sect.m_pSectorDataが指す領域のデータを書き込む
// 指定のセクタがない場合は書き込みを行わない(新規にセクタを作ったりはしない)
// sectの持っているCHRNは使用しない(引数のCHRのみ有効)
FDC_STATUS	CFDImg::WriteSector( int C, int H, int R, CSector *sect ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return 0;
	long Offset;
	DWORD NOW;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;		// Sector not found
	ReadSectorID(Offset, sect);	// SectorIDを読み飛ばし、データ部の頭だしをする
	if(m_fWriteProtect==0x10) return FDC_WRITE_PROTECTED;	// Write Protected!
	if(WriteFile(m_hFDImage, (LPCVOID)sect->m_pSectorData, sect->m_nSectorSize, &NOW, nullptr) == 0) { SHOW_ERROR; }
	return 0;
}

// FD Imageファイルをオープンする。成功した場合、自動的にヘッダ情報も読み込む
bool CFDImg::OpenFDImage( unsigned char *pFileName )
{
	m_hFDImage = CreateFile( (LPCTSTR)pFileName, GENERIC_READ | GENERIC_WRITE, 0,
						nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, nullptr);
	if(m_hFDImage==INVALID_HANDLE_VALUE) {
		SHOW_ERROR;
		return false;
	}
	if(ReadHeader()==false) return false;
	return true;
}

// FD Imageファイルをクローズする
bool CFDImg::CloseFDImage( void ) {
	if(m_hFDImage!=INVALID_HANDLE_VALUE) {
		CloseHandle(m_hFDImage);
		m_hFDImage = INVALID_HANDLE_VALUE;
	}
	return true;
}

// FD Imageのヘッダ情報を読み出す
bool CFDImg::ReadHeader( void ) {
	if(m_hFDImage==INVALID_HANDLE_VALUE) return false;
	DWORD NOR;		// Number Of Read
	SetFilePointer(m_hFDImage, 0, nullptr, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)m_sDiskName,		17, &NOR, nullptr) == 0) { SHOW_ERROR; }
	SetFilePointer(m_hFDImage, 17+9, nullptr, FILE_BEGIN);
	if(ReadFile(m_hFDImage, (LPVOID)&m_fWriteProtect,	1, &NOR, nullptr)==0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&m_fDiskType,		1, &NOR, nullptr)==0) { SHOW_ERROR; }
	if(ReadFile(m_hFDImage, (LPVOID)&m_nDiskSize,		4, &NOR, nullptr)==0) { SHOW_ERROR; }
	for(int i=0; i<=_CFDIMAGE_MAX_TRACK_; i++) {
		if(ReadFile(m_hFDImage, (LPVOID)&m_nTrackOffset[i],	4, &NOR, nullptr)==0) { SHOW_ERROR; }
	}
	return true;
}

// ＦＤイメージデータのヘッダ情報をコンソールに表示する
// fShowTrackOfsetにtrueを指定すると、全トラックのオフセット値も表示する
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
