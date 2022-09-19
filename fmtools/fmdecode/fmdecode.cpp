#include <stdio.h>
#include <stdlib.h>

#include "cmotorolas.h"

#include <iostream>
#include <fstream>

// v0.1 "iba"拡張子を受け付けるように変更

#define VERSION		"v0.1"

class CFMDECODE {
protected:
	std::ifstream hInFile;
	std::ofstream hOutFile;	// 入出力ファイルへのハンドル
	char infile[512], outfile[512];			// 出力ファイル名

	bool fRaw;					// RAWモードフラグ

	char filename[9];
	unsigned char FileType;
	bool Ascii;
	bool Random;
	bool fVerbose;

protected:
	void DecodeBasicFile( void ) {
		unsigned char buff[4];
		hInFile.read(reinterpret_cast<char*>(buff), 2);		// Dummy read (skip 2 bytes)
		while(1) {
			hInFile.read(reinterpret_cast<char*>(buff), 1);
			if(!hInFile.fail()) {
				hOutFile.write(reinterpret_cast<char*>(buff), 1);
				if(Ascii==true) {
					if(buff[0]==0x1a) break;
				}
			} else break;
		}
	}

	void DecodeRawFile( void ) {
		unsigned char buff[4];
		while(1) {
			hInFile.read(reinterpret_cast<char*>(buff), 1);
			if(!hInFile.fail()) {
				hOutFile.write(reinterpret_cast<char*>(buff), 1);
			} else break;
		}
	}

	void DecodeAsciiFile( void ) {
		unsigned char buff[4];
		while(1) {
			hInFile.read(reinterpret_cast<char*>(buff), 1);
			if(!hInFile.fail()) {
				hOutFile.write(reinterpret_cast<char*>(buff), 1);
				if(Ascii==true) {
					if(buff[0]==0x1a) break;
				}
			} else break;
		}
	}

	void DecodeMachineFile( void )
	{
		unsigned char buff[4];
		unsigned long nFileSize;
		unsigned short nStartAddr, nEntryAddr;
		hInFile.read(reinterpret_cast<char*>(buff), 3);
		nFileSize = (buff[0]<<16) | (buff[1]<<8) | buff[2];
//		printf("File size=%06x\n", nFileSize);
		hInFile.read(reinterpret_cast<char*>(buff), 2);
		nStartAddr = (buff[0]<<8) | buff[1];
		char *data = new char[nFileSize+1];
		// データ本体を読み出す
		hInFile.read(reinterpret_cast<char*>(data), nFileSize);
		if(hInFile.fail()) {
			puts("Unexpected EOF");
			delete []data;
			exit(1);
		}
		// Dummyを読み飛ばす(この３バイトは何のためにあるんだろう?)
		hInFile.read(reinterpret_cast<char*>(buff), 3);
		if(hInFile.fail()) {
			puts("Unexpected EOF(1)");
			delete []data;
			exit(1);
		}
		// エントリーアドレスを読み出す
		hInFile.read(reinterpret_cast<char*>(buff), 2);
		if(hInFile.fail()) {
			puts("Unexpected EOF(1)");
			delete []data;
			exit(1);
		}
		nEntryAddr = (buff[0]<<8) | buff[1];
		if(fVerbose) 
			printf("StartAddr=%04x EndAddr=%04x Entry=%04x\n", 
				nStartAddr, nStartAddr+nFileSize-1, nEntryAddr);

		// モトローラSフォーマットでデータを出力
		CMotorolaS mot;
		char outbuff[512];
		// データレコードを生成
		for(unsigned int adr=0; adr<nFileSize; adr+=0x20) {
			mot.InitSRecord(nStartAddr+adr);
			for(unsigned int ofst=0; ofst<0x20; ofst++) {
				if(adr+ofst>=nFileSize) break;
				mot.AddData(data[adr+ofst]);
			}
			mot.GetSRecordBuff(outbuff);
			strcat(outbuff, "\n");
			hOutFile.write(reinterpret_cast<char*>(outbuff), strlen(outbuff));
//			puts(outbuff);
		}
		// エンドレコードを生成
		mot.InitSRecord(nEntryAddr);
		mot.SetRecordType(9);
		mot.GetSRecordBuff(outbuff);
		strcat(outbuff, "\n");
		hOutFile.write(reinterpret_cast<char*>(outbuff), strlen(outbuff));
//		puts(outbuff);
		delete []data;
	}

	bool ReadHeader( void ) {
		unsigned char header[0x10];
		hInFile.read(reinterpret_cast<char*>(header), 0x10);
		if(header[13]!='X' || header[14]!='M' || header[15]!='7') return false;
		// ファイル名抜き取り
		for(int i=0; i<9; i++) {
			filename[i] = header[i];
		}
		FileType = header[10];
		Ascii  = (header[11]==0?false:true);
		Random = (header[12]==0?false:true);
		return true;
	}

	// ヘッダ情報を表示する
	void ShowHeader( void ) {
		if(fVerbose) 
			printf("FileName=%-8s FileType=%d Ascii=%c Random=%c\n", filename, FileType, Ascii?'A':'B', Random?'R':'S');
	}

public:

	CFMDECODE() {
		fRaw = true;
		fVerbose=false;
	}

	void Usage( void )	{
		puts("FM-FILE to DOS/V file decoder " VERSION);
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMDECODE FM_FILE [options]");
		puts("Options: -v  Verbose mode");
		puts("e.g. FMDECODE test.0a0\n");
		puts("Note: The FM_FILE must be created by FMREAD command.");
		puts("Output filename will be genarated automatically");
		puts("  0 A 0 file -> .BAS");
		puts("  1 A 0 file -> .TXT");
		puts("  2 B 0 file -> .MOT");
		puts(" Other  file -> .IMG");
	}
	
	void Ignition( int argc, char*argv[]) {
		if(argc<2) {
			Usage();
			exit(1);
		}
		
		strncpy(infile , argv[1], 510);	// 入力ファイル(FM形式)

		// オプション解析
		for(int i=2; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'v':
				case 'V':
					fVerbose=true;
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
			printf("Failed to open input file '%s'\n", infile);
			exit(1);
		}
		if(!ReadHeader()) {
			printf("'%s' is not a XM7 file\n", infile);
			exit(1);
		}

		// 出力ファイル名を生成する
		// ファイル属性から拡張子を自動生成。
		strcpy(outfile, filename);
		char ext[10];
		strcpy(ext, ".img");
		switch(FileType) {
		case 0:
			if(Ascii!=0)	strcpy(ext, ".bas");
			else			strcpy(ext, ".iba");
			break;
		case 1:
			strcpy(ext, ".txt");
			break;
		case 2:
			strcpy(ext, ".mot");
			break;
		}
		strcat(outfile, ext);
		if(fVerbose) printf("Output file='%s'\n", outfile);

		// 出力ファイルオープン
		hOutFile.open(outfile, std::ios::out | std::ios::trunc);
		if(!hOutFile.fail()) {
			printf("Failed to open output file '%s'\n", outfile);
		//	exit(1);
		}

		ShowHeader();

		switch(FileType) {
		case 0:
			if(Ascii==true)		DecodeBasicFile();
			else				DecodeRawFile();
			break;
		case 1:
			DecodeAsciiFile();
			break;
		case 2:
			DecodeMachineFile();
			break;
		case 9:
			// -rオプション指定時
//			DecodeRawFile();
			break;
		default:
			break;
		}
		hInFile.close();
		hOutFile.close();
	}
};


void main(int argc, char *argv[])
{
	CFMDECODE fmdecode;
	fmdecode.Ignition(argc, argv);
}
