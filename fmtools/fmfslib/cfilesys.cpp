#define FMFSLIB_EXPORTS

#include "cfilesys.h"

bool FDHANDLE::nRefCountUp( void ) {
	if(nRefCount<1024) {
		nRefCount++;
		return true;
	}
	return false;
}

bool FDHANDLE::nRefCountDown( void ) {
	if(nRefCount>0) {
		nRefCount--;
		return true;
	}
	return false;
}

int FDHANDLE::GetRefCount( void ) { return nRefCount; }

//---------------------------------------------------------

inline int	CFilesys::LBA2C( int LBA ) {
	return (LBA/(SPT*HPT));
}

inline int CFilesys::LBA2H( int LBA ) {
	return (LBA/SPT)&1;
}

inline int CFilesys::LBA2R( int LBA ) {
	return LBA-(LBA/SPT)*SPT+1;
}

inline int CFilesys::CHR2LBA( int C, int H, int R ) {
	return (C*HPT+H)*SPT+R-1;
}

inline int CFilesys::Cluster2LBA( int nCluster ) {
	return nCluster*SPC+CLUSTER_TOP_SECTOR;
}

bool CFilesys::ReadCluster( FLHANDLE *hFile, int nCluster ) {
	if(nCluster<0 || nCluster>MAX_CLUSTER) return false;
	CSector csect;
	// FATの値をチェックして使用されているクラスタかどうか確かめる
	int FATValue = GetFATValue(hFile->phFD, nCluster);
	if(FATValue>=0xfd) return false;
	// クラスタ内で使用されているセクタ数を求める
	int UsedSect;
	if(FATValue>=0xc0 && FATValue<=0xc0+SPC) {
		UsedSect = FATValue - 0xbf;		// 中途半端な場合
	} else {
		UsedSect = SPC;					// クラスタを使い切っている場合
	}
	hFile->nClusterDataSize = 0;
	csect.m_pSectorData = (unsigned char*)hFile->pClusterBuff;
	size_t numRead;
	for(int sect=0; sect<UsedSect; sect++) {
		int sctno = Cluster2LBA(nCluster)+sect;
		hFile->phFD->pFD->ReadSector(LBA2C(sctno), LBA2H(sctno), LBA2R(sctno), csect, numRead);
		csect.m_pSectorData += SECT_SIZE;
		hFile->nClusterDataSize += SECT_SIZE;
	}
	hFile->nClusterNo = nCluster;
	hFile->nClusterPtr = 0;
	return true;
}

// クラスタバッファのデータを指定のクラスタ番号のクラスタに書き込む
// FATは操作しない
bool CFilesys::WriteCluster( FLHANDLE *hFile, int nCluster ) {
	if(nCluster<0 || nCluster>MAX_CLUSTER) return false;
	if(hFile->nClusterPtr==0) return true;
	if(hFile->nClusterPtr<0) return false;
	char *ptr = (char*)hFile->pClusterBuff;
	int UsedSect;
	UsedSect = (hFile->nClusterPtr-1)/SECT_SIZE+1;
	CSector csect;
	for(int sect=0; sect<UsedSect; sect++) {
		int sctno = Cluster2LBA(nCluster)+sect;
		csect.m_pSectorData = (unsigned char*)ptr;
		hFile->phFD->pFD->WriteSector(LBA2C(sctno), LBA2H(sctno), LBA2R(sctno), csect);
		ptr += SECT_SIZE;
	}
	return true;
}

bool CFilesys::ReadNextCluster( FLHANDLE *hFile ) {
	int next = GetFATValue(hFile->phFD, hFile->nClusterNo);
	if(next<0 || next>MAX_CLUSTER) return false;
	return ReadCluster(hFile, next);
}

bool CFilesys::ReadFAT( FDHANDLE *hFD, char *pBuff ) {
    bool err;
	CSector tmpsect;
	size_t numRead;
    err=true;
    for(int i=0; i<NUMBER_OF_FAT; i++) {
		tmpsect.m_pSectorData = (unsigned char*)(pBuff+i*SECT_SIZE);
		if(hFD->pFD->ReadSector(LBA2C(FAT_LBA+i), LBA2H(FAT_LBA+i),
				LBA2R(FAT_LBA+i), tmpsect, numRead)!=0) err=false;
    }
	return err;
}

bool CFilesys::WriteFAT( FDHANDLE *hFD, char *pBuff ) {
    bool err;
	CSector tmpsect;
    err=true;
    for(int i=0; i<NUMBER_OF_FAT; i++) {
		tmpsect.m_pSectorData = (unsigned char*)(pBuff+i*SECT_SIZE);
		if(hFD->pFD->WriteSector(LBA2C(FAT_LBA+i), LBA2H(FAT_LBA+i),
				LBA2R(FAT_LBA+i), tmpsect)!=0) err=false;
    }
	return err;
}

int CFilesys::GetFATValue( FDHANDLE *hFD, int cluster ) {
    unsigned char pFAT[SECT_SIZE*NUMBER_OF_FAT];
    if(cluster>MAX_CLUSTER || cluster<0) return -1;
	ReadFAT(hFD, (char*)pFAT);
    return pFAT[cluster+FAT_CLUSTER_OFFSET];
}

bool CFilesys::SetFATValue( FDHANDLE *hFD, int cluster, int value ) {
    unsigned char pFAT[SECT_SIZE*NUMBER_OF_FAT];
    if(cluster>MAX_CLUSTER || cluster<0) return false;
	ReadFAT(hFD, (char*)pFAT);
	pFAT[cluster+FAT_CLUSTER_OFFSET] = value;
    WriteFAT(hFD, (char*)pFAT);
	return true;
}

bool CFilesys::ReadEntry( FDHANDLE *hFD, int EntryNumber, CEntry *ent ) {
    CSector tmpsect;
	size_t numRead;
	unsigned char pSECT[SECT_SIZE];
	tmpsect.m_pSectorData = pSECT;
	int sect = EntryNumber/EPS;
    if(hFD->pFD->ReadSector(LBA2C(DIR_LBA+sect), LBA2H(DIR_LBA+sect),
    		LBA2R(DIR_LBA+sect), tmpsect, numRead)!=0) return false;
	int pos = (EntryNumber-(EntryNumber/EPS)*EPS)*BPE;
    for(int i=0; i<8; i++) {
        ent->pFilename[i] = pSECT[pos++];
    }
    pos+=3;		// Skip Reserve
    ent->nFileType		= pSECT[pos++];
    ent->fAscii			= pSECT[pos++];
    ent->fRandomAccess	= pSECT[pos++];
    ent->nTopCluster	= pSECT[pos++];
	return true;
}

// エントリ情報をディスクに書き込む
bool CFilesys::WriteEntry( FDHANDLE *hFD, int EntryNumber, CEntry *ent ) {
    CSector tmpsect;
	size_t numRead;
    unsigned char pSECT[SECT_SIZE];
	tmpsect.m_pSectorData = pSECT;
	int sect = EntryNumber/EPS;
    if(hFD->pFD->ReadSector(LBA2C(DIR_LBA+sect), LBA2H(DIR_LBA+sect),
    		LBA2R(DIR_LBA+sect), tmpsect, numRead)!=0) return false;
	int pos = (EntryNumber-(EntryNumber/EPS)*EPS)*BPE;
    char fname[20];
	sprintf(fname, "%-8s", ent->pFilename);	// 左詰の8文字のファイル名を生成
	for(int i=0; i<8; i++) pSECT[pos++] = fname[i];
    pos+=3;		// Skip Reserve
    pSECT[pos++]=ent->nFileType;
    pSECT[pos++]=ent->fAscii;
    pSECT[pos++]=ent->fRandomAccess;
    pSECT[pos++]=ent->nTopCluster;
    if(hFD->pFD->WriteSector(LBA2C(DIR_LBA+sect), LBA2H(DIR_LBA+sect),
    		LBA2R(DIR_LBA+sect), tmpsect)!=0) return false;
	return true;
}

// 空のエントリを探し、エントリ番号を返す。見つからないときは-1
int CFilesys::FindEmptyEntry( FDHANDLE *hFD ) {
	CEntry ent;
	for(int entry=0; entry<=MAX_DIR; entry++) {
		ReadEntry(hFD, entry, &ent);
		if((unsigned char)ent.pFilename[0]==0xff) return entry;
	}
	return -1;	
}

// 空きクラスタを探してクラスタ番号を返す
int CFilesys::FindEmptyCluster( FDHANDLE *hFD ) {
	int fv;
	for(int cluster=0; cluster<=MAX_CLUSTER; cluster++) {
		fv=GetFATValue(hFD, cluster);
		if(fv==0xfd || fv==0xff) {
			return cluster;
		}
	}
	return -1;
}

// 指定のファイル名を持つエントリを探す。見つかった場合、エントリ番号を返す
int CFilesys::FindEntry( FDHANDLE *hFD, char *pFilename ) {
	CEntry ent;
	char fname[20];
	sprintf(fname, "%-8s", pFilename);
	for(int entry=0; entry<=MAX_DIR; entry++) {
		int i;
		ReadEntry(hFD, entry, &ent);
		// Compae filename
		for(i=0; i<8; i++) {
			if(fname[i]!=ent.pFilename[i]) break;
		}
		if(i==8) return entry;
	}
	return -1;
}

// C=0,H=0,R=3のセクタを読み込みディスクのIDを確認する
bool CFilesys::CheckDiskID( FDHANDLE *hFD ) {
	bool result=true;
	CSector sect;
	size_t numRead;
	sect.m_pSectorData = new unsigned char [1024];
	hFD->pFD->ReadSector(0, 0, 3, sect, numRead);
	if((sect.m_pSectorData[0]!='S' || sect.m_pSectorData[1]!=' ' || sect.m_pSectorData[2]!=' ') &&
	   (sect.m_pSectorData[0]!='S' || sect.m_pSectorData[1]!='Y' || sect.m_pSectorData[2]!='S')) 
		result=false;	// Illegal Disk ID
	delete []sect.m_pSectorData;
	return result;
}

// 指定のファイル名のファイルをマウントし、FDハンドル値を返す
FDHANDLE *CFilesys::FMMountFD( std::string fdimage ) {
	FDHANDLE *ptr = new FDHANDLE();
	if(ptr==nullptr) return nullptr;

	CFDImg *pFDImg = new CFDImg;
	if(pFDImg->OpenFDImage(fdimage)==false) {
		delete pFDImg;
		return nullptr;
	}
    ptr->pFD = pFDImg;
	if(CheckDiskID(ptr)==false) {		// F-BASICディスクかどうかチェックする
		SetError(FM_NOT_A_FBASIC_DISK);
		delete ptr;
		return nullptr;
	}
	return ptr;
}

// 指定のFDハンドル値を持つFDイメージをアンマウントする
bool CFilesys::FMUnmountFD( FDHANDLE *hFD ) {
    if(hFD==nullptr) return false;
    if(hFD->pFD==nullptr) return false;
	delete hFD->pFD;			// FDイメージを削除
    delete hFD;					// FDハンドルを削除
	return true;
}

// ファイルをオープンする
// リードのみの時はfiletype, ascii, randomの値は使用しない
FLHANDLE *CFilesys::FMOpen( FDHANDLE *hFD, char *filename, int mode, int filetype,
						   int ascii, int random) {
	if(hFD==nullptr) return nullptr;
	if((mode&FM_OPEN_READ)!=0 && (mode&FM_OPEN_WRITE)!=0) return nullptr;	// READ/WRITEを同時に指定は出来ない
	FLHANDLE *hFile = new FLHANDLE();
	hFile->fOpenMode = mode;
	hFile->phFD = hFD;
	memcpy(hFile->pFilename, filename, 8);
	int entryno = FindEntry(hFD, filename);		// エントリは存在するかチェック
	// 指定のエントリが見つからなかったとき
	if(entryno==-1) {
		if((mode & FM_OPEN_READ)!=0) {
			// リードモード時はエラー
			m_Error = 1;
			delete hFile;
			return nullptr;
		} else {
			// ライトモード時は新しいエントリーを作成する
			CEntry ent;
			entryno = FindEmptyEntry(hFD);
			if(entryno==-1) {
				m_Error = 2;
				delete hFile;
				return nullptr;
			}
			int emptycluster = FindEmptyCluster(hFD);
			if(emptycluster==-1) {
				m_Error = 2;
				delete hFile;
				return nullptr;
			}
			SetFATValue(hFD, emptycluster, 0xc0);	// クラスタを使用中にする
			sprintf(ent.pFilename, "%-8s", filename);
			ent.nTopCluster = emptycluster;
			ent.nFileType = filetype;
			ent.fAscii = ascii;
			ent.fRandomAccess = random;
			WriteEntry(hFD, entryno, &ent);			// エントリの書き出し
			hFile->nClusterNo = emptycluster;
			hFile->nClusterPtr = 0;
			hFile->fAscii = ascii;
			hFile->nFileType = filetype;
			hFile->fRandom = random;
		}
	} else {
		// エントリーが既に存在するとき
		CEntry ent;
		ReadEntry(hFD, entryno, &ent);				// エントリ情報の読み出し
		hFile->nClusterNo = ent.nTopCluster;		// 先頭クラスタ番号を調べる
		if((mode & FM_OPEN_READ)!=0) {
			ReadCluster(hFile, hFile->nClusterNo);	// リード時は最初のクラスタまで読み込んでおく
		} else {
			DeleteChain(hFD, hFile->nClusterNo);	// ライト時は既存のクラスタチェーンを破棄(消去)
		}
		hFile->nClusterPtr = 0;						// クラスタ内データポインタ0にする
		hFile->fAscii = ent.fAscii;
		hFile->nFileType = ent.nFileType;
		hFile->fRandom = ent.fRandomAccess;
	}
	hFile->lFilePtr = 0L;							// ファイルポインタを0にする
	hFile->fEOF = false;
	return hFile;
}

bool CFilesys::FMClose( FLHANDLE *hFile ) {
	if(hFile==nullptr) return false;
	if(hFile->nClusterPtr>0 && (hFile->fOpenMode & FM_OPEN_WRITE)!=0) {
		FMFlush(hFile);
//		WriteCluster(hFile, hFile->nClusterNo);
//		SetFATValue(hFile->phFD, hFile->nClusterNo, 0xc0+hFile->nClusterPtr/SECT_SIZE+1);

	}
	delete hFile;
	hFile = nullptr;
	return true;
}

bool CFilesys::FMSeek( FLHANDLE *hFile, long ptr, int origin ) {
	if(hFile==nullptr) return false;
	return true;
}

long CFilesys::FMTell( FLHANDLE *hFile ) {
	if(hFile==nullptr) return -1;
	return hFile->lFilePtr;
}

int CFilesys::Read1( FLHANDLE *hFile ) {
	if(hFile==nullptr) return -1;
	if(hFile->fEOF==true) return -1;
	if(hFile->nClusterPtr>=hFile->nClusterDataSize /*SECT_SIZE*SPC*/) {
		// クラスタポインタがクラスタサイズを超えているときは、次のクラスタを読み込む
		if(ReadNextCluster(hFile)==false) {
			// 次のクラスタを読み込めなかったときはEOFフラグを立てる
			hFile->fEOF = true;
			return -1;
		}
		hFile->nClusterPtr = 0;
	}
	int result;
	result = (unsigned char)(hFile->pClusterBuff[hFile->nClusterPtr++]);
	// 文字が0x1a(ESC)かつ、アスキーモードオープンのときはEOFフラグを立てる
	if(result==FM_EOF_CODE && hFile->fAscii==FM_ASCII) {
		hFile->fEOF = true;
	}
	return result;
}

//#include <stdio.h>

int CFilesys::FMRead( FLHANDLE *hFile, char *pBuff, int nNOR ) {
	int NOR = 0;
	if(hFile==nullptr) return -1;
	for(NOR=0; NOR<nNOR; NOR++) {
		if(hFile->fEOF==true) break;
		int ch = Read1(hFile);
		if(ch<0) {
			hFile->fEOF=true;
			break;
		}
		*pBuff++ = ch;
		if(hFile->fEOF==true) break;
	}
	return NOR;
}

// クラスタバッファの中身をフラッシュする
bool CFilesys::FMFlush( FLHANDLE *hFile ) {
	if(hFile==nullptr) return false;
	if(hFile->nClusterPtr<0) return false;
	if(hFile->nClusterPtr>0) {
		WriteCluster(hFile, hFile->nClusterNo);	// クラスタバッファにデータがあるときは書き込む
//		hFile->nClusterPtr = 0;					// データポインタは巻き戻さない！　追加のデータがあるかもしれないので。
	}
	SetFATValue(hFile->phFD, hFile->nClusterNo,
		(hFile->nClusterPtr-1)/SECT_SIZE+0xc0);		// 一応FATチェーンを閉じる
	return true;
}

bool CFilesys::Write1( FLHANDLE *hFile, char ch ) {
	if(hFile==nullptr) return false;
	if(hFile->nClusterPtr>=SECT_SIZE*SPC) {
		// クラスタバッファがいっぱいだった場合
		WriteCluster(hFile, hFile->nClusterNo);
		int nextcluster = FindEmptyCluster(hFile->phFD);
		if(nextcluster>=0) {
			// 次の空きクラスタが見つかった場合
			SetFATValue(hFile->phFD, hFile->nClusterNo, nextcluster);	// 検索した次の空きクラスタ番号を書き込む
			hFile->nClusterNo = nextcluster;
			hFile->nClusterPtr = 0;
			SetFATValue(hFile->phFD, hFile->nClusterNo, 0xc0);			// 次に使用するクラスタを使用済みにして抑えておく
		} else {
			// 次の空きクラスタが見つからなかった場合
			SetFATValue(hFile->phFD, hFile->nClusterNo, 0xc0+SPC);		// このクラスタで終わりとする
			SetError(FM_NO_DISK_SPACE);
			return false;
		}
	}
	hFile->pClusterBuff[hFile->nClusterPtr++] = ch;
	return true;
}

int CFilesys::FMWrite( FLHANDLE *hFile, char *pBuff, int nNOW ) {
	if(hFile==nullptr) return -1;
	int NOW = 0;
	for(int i=0; i<nNOW; i++) {
		if(Write1(hFile, *pBuff++)==false) return NOW;
		NOW++;
	}
	return NOW;
}

bool CFilesys::FMEof( FLHANDLE *hFile ) {
	if(hFile==nullptr) return false;
	return hFile->fEOF;
}

// 最初のエントリの内容を返す
bool CFilesys::FMGetFirstEntry( FDHANDLE *hFD, CEntry *ent ) {
	if(hFD==nullptr) return false;
	m_nEntryCount = 0;
	ReadEntry(hFD, m_nEntryCount, ent);
	m_nEntryCount++;
	return true;
}

// エントリの内容を返す
bool CFilesys::FMGetNextEntry( FDHANDLE *hFD, CEntry *ent ) {
	if(hFD==nullptr) return false;
	ReadEntry(hFD, m_nEntryCount, ent);
	m_nEntryCount++;
	return true;
}

FMERR CFilesys::FMGetLastError( void ) {
	return m_Error;
}

void CFilesys::SetError( FMERR nError ){
	m_Error = nError;
};

// 指定のクラスタ番号から始まるFATチェーンを削除
bool CFilesys::DeleteChain( FDHANDLE *hFD, int nCluster ) {
	if(hFD==nullptr) return false;
	if(nCluster<0 || nCluster>MAX_CLUSTER) return false;
	int nextcluster, cluster = nCluster;
	do {
		nextcluster = GetFATValue(hFD, cluster);
		SetFATValue(hFD, cluster, 0xff);	// クラスタ開放
		cluster = nextcluster;
	} while(cluster<=MAX_CLUSTER);
	return true;
};

// 指定のファイルを削除
bool CFilesys::FMDeleteFile( FDHANDLE *hFD, char *filename ) {
	if(hFD==nullptr) return false;
	CEntry ent;
	int entno;
	entno = FindEntry(hFD, filename);
	if(entno<-1) {
		SetError(FM_FILE_NOT_FOUND);
		return false;
	}
	ReadEntry(hFD, entno, &ent);
	ent.pFilename[0] = (char)0xff;		// エントリ消去(ファイル名の先頭の1バイトを0xffにする)
	WriteEntry(hFD, entno, &ent);
	return DeleteChain(hFD, ent.nTopCluster); // FATチェーンを削除
};

bool CFilesys::FMGetFileInfo( FDHANDLE *hFD, char *filename, CEntry *ent ) {
	return ReadEntry(hFD, FindEntry(hFD, filename), ent);
}
