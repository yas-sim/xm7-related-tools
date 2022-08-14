#ifndef _CFILESYS_H_
#define _CFILESYS_H_

#include "cfdimg.h"
#include "fmerr.h"

#define FM_OPEN_READ	(1<<0)
#define FM_OPEN_WRITE	(1<<1)
#define FM_SEEK_BEGIN	(1<<0)
#define FM_SEEK_BOTTOM	(1<<1)
#define FM_SEEK_CURRENT	(1<<2)

#define FM_BASIC_SRC	(0x00)
#define FM_BASIC_DATA	(0x01)
#define FM_MACHINE		(0x02)
#define FM_ASCII		(0xff)
#define FM_BINARY		(0x00)
#define FM_SEQUENTIAL	(0x00)
#define FM_RANDOM		(0xff)
#define FM_EOF_CODE		(0x1a)

#define FAT_LBA				(32)	// FATのスタートLBA番号
#define NUMBER_OF_FAT		(1)		// FATで使うセクタ数
#define SECT_SIZE			(256)	// 1セクタのサイズ
#define MAX_CLUSTER			(151)	// クラスタ番号の最大値
#define SPC					(8)		// Sector/Cluster
#define FAT_CLUSTER_OFFSET	(5)		// FATの何バイト目からが第０クラスタに対応するか
#define DIR_LBA				(35)	// ディレクトリエントリの最初のセクタ番号(LBA)
#define MAX_DIR				(231)	// エントリの最大数
#define BPE					(32)	// Byte/Entry
#define SPT					(16)	// Sector/Track(Cylinder)
#define HPT					(2)		// Head/Track(Cylinder)
#define EPS					(SECT_SIZE/BPE)	// １セクタ内におさまるディレクトリエントリ数
#define CLUSTER_TOP_SECTOR	(64)	// 第０クラスタの先頭セクタ番号(LBA)

typedef int FMERR;			// Error codeの型

class FMFSLIB_API FDHANDLE {
protected:
	int nRefCount;		// 参照数
public:
	CFloppy *pFD;
public:
	FDHANDLE() {
		pFD = nullptr;
		nRefCount = 0;
	}
	virtual ~FDHANDLE() {
	}
	bool nRefCountUp( void );
	bool nRefCountDown( void );
	int GetRefCount( void );
};

class FMFSLIB_API FLHANDLE {
public:
	FDHANDLE *phFD;
	char pFilename[9];
	long lFilePtr;
	int fOpenMode;
	int fAscii;
	int fRandom;
	int nFileType;
	bool fEOF;
	char pClusterBuff[SECT_SIZE*SPC];	// クラスタバッファ
	int nClusterNo;						// 現在のクラスタバッファのクラスタ番号
	int nClusterPtr;					// クラスタ内のR/Wポインタ
	int nClusterDataSize;				// 読み込んだクラスタのサイズ(クラスタを使い切ってない場合にはクラスタサイズより小さくなる)
public:
	FLHANDLE() {
		phFD = nullptr;
		pFilename[8] = '\0';
		lFilePtr = 0L;
		fOpenMode = 0;
		bool fEOF = false;
		nClusterNo = -1;
		nClusterPtr = 0;
		nClusterDataSize = 0;
		fAscii = false;
		fRandom = false;
		nFileType = 0;
	}
	virtual ~FLHANDLE() {
	}
};

class FMFSLIB_API CEntry {
public:
	char	pFilename[9];
    unsigned char	nFileType;			// 00:BASICソース  01:BASICデータ  02:機械語ファイル
    unsigned char	fAscii;				// 00:BINARY    FF:ASCII
    unsigned char	fRandomAccess;
    unsigned char	nTopCluster;
	CEntry() {
		pFilename[8] = '\0';}
};

class FMFSLIB_API CFilesys {
protected:
	FMERR	m_Error;
	int		m_nEntryCount;		// FMGetFirstFilenameとFMGetNextFilenameで使用
protected:
	inline int LBA2C( int LBA );
	inline int LBA2H( int LBA );
	inline int LBA2R( int LBA );
	inline int CHR2LBA( int C, int H, int R );
	inline int Cluster2LBA( int nCluster );

	bool CheckDiskID( FDHANDLE *hFD );
	bool ReadFAT( FDHANDLE *hFD, char *pBuff );
	bool WriteFAT( FDHANDLE *hFD, char *pBuff );
	bool SetFATValue( FDHANDLE *hFD, int cluster, int value );
	int FindEmptyCluster( FDHANDLE *hFD );
	int FindEntry( FDHANDLE *hFD, char *pFilename );
	int FindEmptyEntry( FDHANDLE *hFD );
	bool ReadEntry( FDHANDLE *hFD, int EntryNumber, CEntry *ent );
	bool WriteEntry( FDHANDLE *hFD, int EntryNumber, CEntry *ent );
	bool ReadCluster( FLHANDLE *hFile, int nCluster );
	bool WriteCluster( FLHANDLE *hFile, int nCluster );
	bool ReadNextCluster( FLHANDLE *hFile );
	int Read1( FLHANDLE *hFile );
	bool Write1( FLHANDLE *hFile, char ch );
	void SetError( FMERR nError );
	bool DeleteChain( FDHANDLE *hFD, int nCluster );
public:
	CFilesys() {
		m_Error = 0;
		m_nEntryCount = 0;
	}
	virtual ~CFilesys() {};
public:
	FDHANDLE*	FMMountFD( char *fdimage );
	bool		FMUnmountFD( FDHANDLE *hFD ); 
	FLHANDLE*	FMOpen( FDHANDLE *hFD, char *filename, int mode, int filetype, int ascii, int random);
	bool		FMClose( FLHANDLE *hFile );
	bool		FMSeek( FLHANDLE *hFile, long ptr, int origin );
	long		FMTell( FLHANDLE *hFile );
	int			FMRead( FLHANDLE *hFile, char *pBuff, int nNOR );
	int			FMWrite( FLHANDLE *hFile, char *pBuff, int nNOW );
	bool		FMEof( FLHANDLE *hFile );
	bool		FMGetFirstEntry( FDHANDLE *hFD, CEntry *ent );
	bool		FMGetNextEntry( FDHANDLE *hFD, CEntry *ent );
	FMERR		FMGetLastError( void );
	bool		FMDeleteFile( FDHANDLE *hFD, char *filename );
	bool		FMFlush( FLHANDLE *hFile );
	bool		FMGetFileInfo( FDHANDLE *hFD, char *filename, CEntry *ent );

	int 		GetFATValue( FDHANDLE *hFD, int cluster );
};

#endif // _CFILESYS_H_
