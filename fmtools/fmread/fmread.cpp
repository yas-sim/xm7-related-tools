#include <stdio.h>
#include <stdlib.h>
#include <CFDImg.h>
#include <CFilesys.h>

#define VERSION  "v0.0"

class CFMREAD {
protected:
	CFilesys fs;
	char tgtfile[10], fdimage[512];

	HANDLE hOutFile;			// �o�̓t�@�C���ւ̃n���h��
	char outfile[20];			// �o�̓t�@�C����

	CEntry ent;
	FLHANDLE *hFile;			// ���̓t�@�C���ւ̃n���h��(F-BASIC�t�@�C���V�X�e��)
	FDHANDLE *hFD;				// ����FD�C���[�W�t�@�C���̃n���h��
	bool fRaw;					// RAW���[�h�t���O

	bool fVerbose;
protected:

	void ReadRawFile( void ) {
		unsigned char buff[4];
		DWORD NOW;
		hFile->fAscii = 0;	// �����I�Ƀo�C�i�����[�h�ɂ���
		while(1) {
			int nor = fs.FMRead(hFile, (char*)buff, 1);
			if(nor>0) {
				WriteFile(hOutFile, &buff, 1, &NOW, NULL);
//				printf("%c", buff[0]);
			}
			if(fs.FMEof(hFile)==true) break;
		}
	}

	// �w�b�_������������
	void WriteHeader( HANDLE hFile, char *filename, unsigned char FileType,
								unsigned char Ascii, unsigned char Random )  {
		char header[0x10];
		DWORD NOW, ptr;
		// �w�b�_��0�N���A
		for(int i=0; i<0x10; i++) header[i]=0;
		ptr=0;
		// �t�@�C��������
		for(i=0; i<9; i++) {
			header[ptr++] = filename[i];
			if(filename[i]=='\0') break;
		}
		header[10] = FileType;
		header[11] = Ascii;
		header[12] = Random;
		// ���ʗp�}�W�b�N�i���o�[
		header[13] = 'X';
		header[14] = 'M';
		header[15] = '7';
		WriteFile(hFile, header, 0x10, &NOW, NULL);
	}

public:

	CFMREAD() {
		fRaw = false;
		fVerbose = false;
	}

	void Usage( void )	{
		puts("File extractor for F-BASIC formatted D77 Disk image " VERSION);
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMREAD FDIMAGE_NAME FILENAME [options]");
		puts("Options: -v  Verbose mode");
		puts("e.g. FMREAD disk.d77 test");
	}

	void Ignition( int argc, char*argv[]) {
		if(argc<3) {
			Usage();
			exit(1);
		}
		
		strncpy(fdimage, argv[1], 510);	// �f�B�X�N�C���[�W�t�@�C����
		strncpy(tgtfile, argv[2], 9);	// �f�B�X�N�C���[�W���̃t�@�C����

		// �I�v�V�������
		for(int i=3; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'h':
				case 'H':
				case '?':
					Usage();
					break;
				case 'V':
				case 'v':
					fVerbose=true;
					break;
				default:
					Usage();
					break;
				}
				break;
			default:
				break;
			}
		}

		hFD = fs.FMMountFD(fdimage);
		if(!fs.FMGetFileInfo(hFD, tgtfile, &ent)) {
			puts("File not found");
			exit(1);
		}
		hFile = fs.FMOpen(hFD, tgtfile, FM_OPEN_READ, 0, 0, 0);
		if(hFile==NULL) {
			puts("Faild to open file");
			exit(-1);
		}

		// �o�̓t�@�C�������쐬
		sprintf(outfile, "%s.%d%c%d", tgtfile,
			ent.nFileType,					// �t�@�C���^�C�v 0,1,2
			ent.fAscii==0x00?'B':'A',		// �A�X�L�[�t���O
			ent.fRandomAccess==0x00?0:1);
		if(fVerbose) {
			printf("Output file='%s'\n", outfile);
		}

		hOutFile = CreateFile(outfile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
		if(hOutFile==INVALID_HANDLE_VALUE) {
			puts("Failed to open output file");
			exit(1);
		}

		WriteHeader(hOutFile, tgtfile, ent.nFileType, ent.fAscii, ent.fRandomAccess);
		ReadRawFile();		// �t�@�C���̒��g��ǂݏo��(���׍H�Ȃ�)
		
		CloseHandle(hOutFile);
		fs.FMClose(hFile);
		fs.FMUnmountFD(hFD);
	}
};

void main(int argc, char *argv[])
{
	CFMREAD fmread;
	fmread.Ignition(argc, argv);
}
