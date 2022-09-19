#define FMFSLIB_EXPORTS

#include <iostream>

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
		ReadSectorID(Offset, sect);
		if(sect.m_nC==C && sect.m_nH==H && sect.m_nR==R) break;
		Offset += (sect.m_nSectorSize+16);		// 16 is a size of sector header
	} while(sect_count++ < sect.m_nNumberOfSectors);
	return Offset;
}

// 指定のオフセット位置からセクタのID情報を読み取る
// セクタデータの読み出しは行わない
bool CFDImg::ReadSectorID( long Offset, CSector &sect ) {
	if(!m_hFDImage.is_open()) return 0;
	uint8_t reserve[5];
	m_hFDImage.seekg(Offset, std::ios::beg);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nC), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nH), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nR), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nN), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nNumberOfSectors), 2);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_fDensity), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_fDDM), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nStatus), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&reserve), 5);
	m_hFDImage.read(reinterpret_cast<char*>(&sect.m_nSectorSize), 2);
	return true;
}

// 指定のCHRのセクタを読み出す
// sectのm_pSectorDataが指す領域にデータを書き込む。
// m_pSectorDataには領域を確保しておく必要あり
FDC_STATUS	CFDImg::ReadSector( const int C, const int H, const int R, CSector &sect ) {
	if(!m_hFDImage.is_open()) return 0;
	long Offset;
	uint64_t NOR;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;											// Sector not found
	ReadSectorID(Offset, sect);
	m_hFDImage.read(reinterpret_cast<char*>(sect.m_pSectorData), sect.m_nSectorSize);
	return 0;
}

// 指定のCHRのセクタに、sect.m_pSectorDataが指す領域のデータを書き込む
// 指定のセクタがない場合は書き込みを行わない(新規にセクタを作ったりはしない)
// sectの持っているCHRNは使用しない(引数のCHRのみ有効)
FDC_STATUS	CFDImg::WriteSector( int C, int H, int R, CSector &sect ) {
	if(!m_hFDImage.is_open()) return 0;
	long Offset;
	Offset = GetSectorOffset(C, H, R);
	if(Offset==0) return 1;		// Sector not found
	ReadSectorID(Offset, sect);	// SectorIDを読み飛ばし、データ部の頭だしをする
	if(m_fWriteProtect==0x10) return FDC_WRITE_PROTECTED;	// Write Protected!
	m_hFDImage.write(reinterpret_cast<char*>(sect.m_pSectorData), sect.m_nSectorSize);
	return 0;
}

// FD Imageファイルをオープンする。成功した場合、自動的にヘッダ情報も読み込む
bool CFDImg::OpenFDImage( const std::string &FileName )
{
	m_hFDImage = std::fstream(FileName, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	if(!m_hFDImage.is_open()) {
		std::cout << "Error opening file '" << FileName << "'." << std::endl;
		return false;
	}
	if(ReadHeader()==false) return false;
	return true;
}

// FD Imageファイルをクローズする
bool CFDImg::CloseFDImage( void ) {
	if(m_hFDImage.is_open()) {
		m_hFDImage.close();
	}
	return true;
}

// FD Imageのヘッダ情報を読み出す
bool CFDImg::ReadHeader( void ) {
	if(!m_hFDImage.is_open()) return false;
	m_hFDImage.seekg(0, std::ios::beg);

	uint64_t dmy;
	m_sDiskName.resize(16);
	m_hFDImage.read(const_cast<char*>(m_sDiskName.c_str()), 16);
	m_hFDImage.seekg(17+9, std::ios::beg);
	m_hFDImage.read(reinterpret_cast<char*>(&m_fWriteProtect), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&m_fDiskType), 1);
	m_hFDImage.read(reinterpret_cast<char*>(&m_nDiskSize), 4);
	for(int i=0; i<_CFDIMAGE_MAX_TRACK_; i++) {
		m_hFDImage.read(reinterpret_cast<char*>(&m_nTrackOffset[i]), 4);
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
