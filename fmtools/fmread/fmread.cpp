#include <stdio.h>
#include <stdlib.h>
#include <CFDImg.h>
#include <CFilesys.h>

#define VERSION  "v0.0"

class CFMREAD {
protected:
	CFilesys fs;
	char tgtfile[10], fdimage[512];

	HANDLE hOutFile;			// 出力ファイルへのハンドル
	char outfile[20];			// 出力ファイル名

	CEntry ent;
	FLHANDLE *hFile;			// 入力ファイルへのハンドル(F-BASICファイルシステム)
	FDHANDLE *hFD;				// 入力FDイメージファイルのハンドル
	bool fRaw;					// RAWモードフラグ

	bool fVerbose;
protected:

	void ReadRawFile( void ) {
		unsigned char buff[4];
		DWORD NOW;
		hFile->fAscii = 0;	// 強制的にバイナリモードにする
		while(1) {
			int nor = fs.FMRead(hFile, (char*)buff, 1);
			if(nor>0) {
				WriteFile(hOutFile, &buff, 1, &NOW, NULL);
//				printf("%c", buff[0]);
			}
			if(fs.FMEof(hFile)==true) break;
		}
	}

	// ヘッダ情報を書き込む
	void WriteHeader( HANDLE hFile, char *filename, unsigned char FileType,
								unsigned char Ascii, unsigned char Random )  {
		char header[0x10];
		DWORD NOW, ptr;
		// ヘッダを0クリア
		for(int i=0; i<0x10; i++) header[i]=0;
		ptr=0;
		// ファイル名複写
		for(int i=0; i<9; i++) {
			header[ptr++] = filename[i];
			if(filename[i]=='\0') break;
		}
		header[10] = FileType;
		header[11] = Ascii;
		header[12] = Random;
		// 識別用マジックナンバー
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
		
		strncpy(fdimage, argv[1], 510);	// ディスクイメージファイル名
		strncpy(tgtfile, argv[2], 9);	// ディスクイメージ内のファイル名

		// オプション解析
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

		// 出力ファイル名を作成
		sprintf(outfile, "%s.%d%c%d", tgtfile,
			ent.nFileType,					// ファイルタイプ 0,1,2
			ent.fAscii==0x00?'B':'A',		// アスキーフラグ
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
		ReadRawFile();		// ファイルの中身を読み出す(小細工なし)
		
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
