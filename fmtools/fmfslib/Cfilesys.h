#ifndef _CFILESYS_H_
#define _CFILESYS_H_

#include "CFDImg.h"
#include "FMERR.H"

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

#define FAT_LBA				(32)	// FAT�̃X�^�[�gLBA�ԍ�
#define NUMBER_OF_FAT		(1)		// FAT�Ŏg���Z�N�^��
#define SECT_SIZE			(256)	// 1�Z�N�^�̃T�C�Y
#define MAX_CLUSTER			(151)	// �N���X�^�ԍ��̍ő�l
#define SPC					(8)		// Sector/Cluster
#define FAT_CLUSTER_OFFSET	(5)		// FAT�̉��o�C�g�ڂ��炪��O�N���X�^�ɑΉ����邩
#define DIR_LBA				(35)	// �f�B���N�g���G���g���̍ŏ��̃Z�N�^�ԍ�(LBA)
#define MAX_DIR				(231)	// �G���g���̍ő吔
#define BPE					(32)	// Byte/Entry
#define SPT					(16)	// Sector/Track(Cylinder)
#define HPT					(2)		// Head/Track(Cylinder)
#define EPS					(SECT_SIZE/BPE)	// �P�Z�N�^���ɂ����܂�f�B���N�g���G���g����
#define CLUSTER_TOP_SECTOR	(64)	// ��O�N���X�^�̐擪�Z�N�^�ԍ�(LBA)

typedef int FMERR;			// Error code�̌^

class FDHANDLE {
protected:
	int nRefCount;		// �Q�Ɛ�
public:
	CFloppy *pFD;
public:
	FDHANDLE() {
		pFD = NULL;
		nRefCount = 0;
	}
	virtual ~FDHANDLE() {
	}
	bool nRefCountUp( void );
	bool nRefCountDown( void );
	int GetRefCount( void );
};

class FLHANDLE {
public:
	FDHANDLE *phFD;
	char pFilename[9];
	long lFilePtr;
	int fOpenMode;
	int fAscii;
	int fRandom;
	int nFileType;
	bool fEOF;
	char pClusterBuff[SECT_SIZE*SPC];	// �N���X�^�o�b�t�@
	int nClusterNo;						// ���݂̃N���X�^�o�b�t�@�̃N���X�^�ԍ�
	int nClusterPtr;					// �N���X�^����R/W�|�C���^
	int nClusterDataSize;				// �ǂݍ��񂾃N���X�^�̃T�C�Y(�N���X�^���g���؂��ĂȂ��ꍇ�ɂ̓N���X�^�T�C�Y��菬�����Ȃ�)
public:
	FLHANDLE() {
		phFD = NULL;
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

class CEntry {
public:
	char	pFilename[9];
    unsigned char	nFileType;			// 00:BASIC�\�[�X  01:BASIC�f�[�^  02:�@�B��t�@�C��
    unsigned char	fAscii;				// 00:BINARY    FF:ASCII
    unsigned char	fRandomAccess;
    unsigned char	nTopCluster;
	CEntry() {
		pFilename[8] = '\0';}
};

class FMFSLIB_API CFilesys {
protected:
	FMERR	m_Error;
	int		m_nEntryCount;		// FMGetFirstFilename��FMGetNextFilename�Ŏg�p
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
