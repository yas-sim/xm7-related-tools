#include <stdio.h>
#include <stdlib.h>
#include <CFDImg.h>
#include <CFilesys.h>

#define VERSION "v0.0"

class CFMCOPY {
protected:
	CFilesys fs;
	char tgtfile[10], infdimage[512], outfdimage[512];

	CEntry ent;

	CFDImg InFD, OutFD;
	FLHANDLE *hInFile;			// 入力ファイルへのハンドル(F-BASICファイルシステム)
	FLHANDLE *hOutFile;			// 入力ファイルへのハンドル(F-BASICファイルシステム)
	FDHANDLE *hInFD, *hOutFD;	// 入出力FDイメージファイルのハンドル

protected:

	bool fVerbose;
public:

	CFMCOPY() {
		fVerbose=false;
	}

	void Usage( void )	{
		puts("File copy tool for F-BASIC formatted D77 FD Image " VERSION );
		puts("Progammed by an XM7 supporter\n");
		puts("Synopsis: FMCOPY In_FD_image Out_FD_Image FM_Filename [OPTIONS]\n");
		puts("Option: -v  Verbose mode\n");
		puts("e.g. FMCOPY input.d77 output.d77 test");
	}

	void Ignition( int argc, char*argv[]) {
		if(argc<4) {
			Usage();
			exit(1);
		}
		
		strncpy(infdimage,  argv[1], 510);	// ディスクイメージファイル名
		strncpy(outfdimage, argv[2], 510);	// ディスクイメージファイル名
		strncpy(tgtfile,    argv[3], 9);	// ディスクイメージ内のファイル名

		// オプション解析
		for(int i=4; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'h':
				case 'H':
				case '?':
					Usage();
					break;
				case 'v':
				case 'V':
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

		char DataBuff[0x10000];
		char *wptr;
//--------------------------------------------------------------
// 入力FDイメージ、ファイルのオープン
		hInFD = fs.FMMountFD(infdimage);
		if(hInFD==false) {
			printf("Failed to mount input FD image file.\n");
		}
		if(!fs.FMGetFileInfo(hInFD, tgtfile, &ent)) {
			puts("File not found");
			exit(1);
		}
		// エントリ情報を取得
		fs.FMGetFileInfo(hInFD, tgtfile, &ent);
		hInFile = fs.FMOpen(hInFD, tgtfile, FM_OPEN_READ, 0, 0, 0);
		if(hInFile==NULL) {
			puts("Faild to open input file");
			exit(-1);
		}
		// ファイルの中身を読み出す
		wptr = DataBuff;
		do {
			if(fs.FMRead(hInFile, wptr, 1)==0) break;
			wptr++;
		} while (fs.FMEof(hInFile)==false);

		fs.FMClose(hInFile);
		fs.FMUnmountFD(hInFD);

//--------------------------------------------------------------
// 出力FDイメージ、ファイルのオープン
		hOutFD = fs.FMMountFD(outfdimage);
		if(hOutFD==NULL) {
			printf("Failed to mount FD image file.\n");
		}
		hOutFile = fs.FMOpen(hOutFD, tgtfile, FM_OPEN_WRITE, ent.nFileType, 
													ent.fAscii, ent.fRandomAccess);
		if(hOutFile==NULL) {
			puts("Faild to open output file");
			exit(-1);
		}
		// ファイル内容の書き込み
		fs.FMWrite(hOutFile, DataBuff, wptr-DataBuff);
		if(fVerbose) printf("%d bytes copied\n", wptr-DataBuff);

		fs.FMClose(hOutFile);
		fs.FMUnmountFD(hOutFD);
	}
};

void main(int argc, char *argv[])
{
	CFMCOPY fmcopy;
	fmcopy.Ignition(argc, argv);
}
