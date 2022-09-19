#include <stdio.h>
#include <stdlib.h>
#include <CFDImg.h>
#include <CFilesys.h>

#define VERSION		"v0.0"

#include "CMotorolaS.h"

#include <iostream>

class CFMWRITE {
protected:
	CFilesys fs;
	char tgtfile[10], fdimage[512];

	std::fstream hInFile;		// 入力ファイルへのハンドル
	char infile[512];			// 入力ファイル名

	CFDImg FD;
	CEntry ent;
	FLHANDLE *hFile;			// 入力ファイルへのハンドル(F-BASICファイルシステム)
	FDHANDLE *hFD;				// 入力FDイメージファイルのハンドル
	bool fRaw;					// RAWモードフラグ

	unsigned char FileType;
	bool Ascii;
	bool Random;

	bool fVerbose;

protected:

	bool ReadHeader( void ) {
		uint32_t NOR;
		unsigned char header[0x10];
		hInFile.read(reinterpret_cast<char*>(header), 0x10);
		bool status = hInFile.fail();
		if(status==true) {
			std::cerr << "Header read error." << std::endl;
			return false;
		}
		if(header[13]!='X' || header[14]!='M' || header[15]!='7') return false;
		// ファイル名抜き取り
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
		while(1) {
			hInFile.read(reinterpret_cast<char*>(&buff), 1);
			bool status = hInFile.fail();
			if(!status) {
				fs.FMWrite(hFile, (char*)buff, 1);
			} else break;
		}
	}

	void Ignition( int argc, char*argv[]) {
		if(argc<3) {
			Usage();
			exit(1);
		}
		
		strncpy(fdimage, argv[1], 510);	// ディスクイメージファイル名
		strncpy(infile , argv[2], 510);	// 入力ファイル名

		// オプション解析
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

		// 入力ファイルオープン
		hInFile.open(infile, std::ios::in | std::ios::binary);
		if(!hInFile.is_open()) {
			puts("Failed to open input file");
			exit(1);
		}
		// ヘッダ情報読み出し
		if(!ReadHeader()) {
			printf("'%s' is not a XM7 format.\n", infile);
			exit(1);
		}

		// FDイメージファイルマウント
		hFD = fs.FMMountFD(fdimage);

		// FMディスク内ファイルオープン
		hFile = fs.FMOpen(hFD, tgtfile, FM_OPEN_WRITE, FileType,
							Ascii==true?0xff:0, Random==true?1:0);
		if(hFile==NULL) {
			puts("Faild to open output file");
			exit(-1);
		}

		// ファイル内容を書き込む
		WriteRawFile();

		hInFile.close();
		fs.FMClose(hFile);
		fs.FMUnmountFD(hFD);
	}
};


void main(int argc, char *argv[])
{
	CFMWRITE fmwrite;
	fmwrite.Ignition(argc, argv);
}
