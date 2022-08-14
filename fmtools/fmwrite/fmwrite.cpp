#include <stdio.h>
#include <stdlib.h>
#include <CFDImg.h>
#include <CFilesys.h>

#define VERSION		"v0.0"

#include "CMotorolaS.h"


class CFMWRITE {
protected:
	CFilesys fs;
	char tgtfile[10], fdimage[512];

	HANDLE hInFile;				// ���̓t�@�C���ւ̃n���h��
	char infile[512];			// ���̓t�@�C����

	CFDImg FD;
	CEntry ent;
	FLHANDLE *hFile;			// ���̓t�@�C���ւ̃n���h��(F-BASIC�t�@�C���V�X�e��)
	FDHANDLE *hFD;				// ����FD�C���[�W�t�@�C���̃n���h��
	bool fRaw;					// RAW���[�h�t���O

	unsigned char FileType;
	bool Ascii;
	bool Random;

	bool fVerbose;

protected:

	bool ReadHeader( void ) {
		DWORD NOR;
		unsigned char header[0x10];
		BOOL status = ReadFile(hInFile, header, 0x10, &NOR, NULL);
		if(status==0) {
			DWORD error = GetLastError();
			char errstr[512];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
				0, errstr, 510, NULL);
		}
		if(header[13]!='X' || header[14]!='M' || header[15]!='7') return false;
		// �t�@�C�����������
		for(int i=0; i<9; i++) {
			tgtfile[i] = header[i];
		}
		FileType = header[10];
		Ascii  = (header[11]==0?false:true);
		Random = (header[12]==0?false:true);
		return true;
	}

public:

	CFMWRITE() {
		fRaw = false;
		fVerbose = false;
	}

	void Usage( void )	{
		puts("File writer for F-BASIC formatted D77 disk image " VERSION );
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMWRITE FDIMAGE_NAME FM-FILE");
//		puts("Options: -v  Verbose mode");
		puts("ex. FMWRITE disk.d77 test");
		puts("The FM-FILE must be created by FMREAD or FMENCODE command");
	}
	
	void WriteRawFile( void ) {
		unsigned char buff[4];
		DWORD NOR;
		hFile->fAscii = 0;	// �����I�Ƀo�C�i�����[�h�ɂ���
		while(1) {
			BOOL status = ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR>0) {
				fs.FMWrite(hFile, (char*)buff, 1);
			} else break;
		}
	}

	void Ignition( int argc, char*argv[]) {
		if(argc<3) {
			Usage();
			exit(1);
		}
		
		strncpy(fdimage, argv[1], 510);	// �f�B�X�N�C���[�W�t�@�C����
		strncpy(infile , argv[2], 510);	// ���̓t�@�C����

		// �I�v�V�������
		for(int i=3; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'v':
				case 'V':
					fVerbose = true;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		// ���̓t�@�C���I�[�v��
		hInFile = CreateFile(infile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
		if(hInFile==INVALID_HANDLE_VALUE) {
			puts("Failed to open input file");
			exit(1);
		}
		// �w�b�_���ǂݏo��
		if(!ReadHeader()) {
			printf("'%s' is not a XM7 format.\n", infile);
			exit(1);
		}

		// FD�C���[�W�t�@�C���}�E���g
		hFD = fs.FMMountFD(fdimage);

		// FM�f�B�X�N���t�@�C���I�[�v��
		hFile = fs.FMOpen(hFD, tgtfile, FM_OPEN_WRITE, FileType,
							Ascii==true?0xff:0, Random==true?1:0);
		if(hFile==NULL) {
			puts("Faild to open output file");
			exit(-1);
		}

		// �t�@�C�����e����������
		WriteRawFile();

		CloseHandle(hInFile);
		fs.FMClose(hFile);
		fs.FMUnmountFD(hFD);
	}
};


void main(int argc, char *argv[])
{
	CFMWRITE fmwrite;
	fmwrite.Ignition(argc, argv);
}
