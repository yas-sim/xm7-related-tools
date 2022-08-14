#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "cmotorolas.h"

// v0.1 001009	"iba"拡張子をBASICバイナリプログラムとして扱うように変更

#define VERSION "v0.1"

class CFMENCODE {
protected:
	HANDLE hInFile, hOutFile;				// 入出力ファイルへのハンドル
	char infile[512], outfile[512];			// 出力ファイル名

	bool fRaw;								// RAWモードフラグ

	char filename[9];
	unsigned char FileType;
	bool Ascii;
	bool Random;

	bool fVerbose;
protected:
	// ファイル名から拡張子部分のみを抜き取る
	void GetFileExtention( char *filename, char *ext) {
		int ln = 0;
		char *ptr = filename;
		while(*ptr) ptr++;								// 文字列の最後を探す 
		while(ptr>=filename && *ptr!='.') ptr--;		// '.'を見つけるか、文字列の先頭にくるまで下がる
		if(*ptr=='.') {
			ptr++;
			while(*ptr) {
				*ext++ = *ptr++;						// '.'が見つかった場合、extにptr+1以降を複写
				if(ln++>=3) break;						// 拡張子が３文字を超えたらループを抜ける
			}
		}
		*ext = '\0';									// 文字列をターミネートする
	}

	// ファイル名中のベースファイル名部分を抜き出す
	void GetFileBase( char *filename, char *base) {
		int ln = 0;
		while(*filename!='\0' && *filename!='.') {
			*base++ = *filename++;
			if(ln++>=8) break;
		}
		*base = '\0';
	}

	// 拡張子を解析して、ファイルタイプを決定する
	bool AnalyzeFileExtention( char *ext, unsigned char *FileType, bool *Ascii, bool *Random) {
		// デフォルトファイルタイプを設定しておく
		*FileType = 1;
		*Ascii = true;
		*Random = false;
		if(strlen(ext)<=3) {
			// 拡張子の長さが３以下のときのみ解析する
			_strupr(ext);
			if(strcmp(ext, "TXT")==0) {
				*FileType	= 1;
				*Ascii		= true;
			}
			if(strcmp(ext, "MOT")==0 || strcmp(ext,"S")==0) {
				*FileType	= 2;
				*Ascii		= false;
			}
			if(strcmp(ext, "BAS")==0) {
				*FileType	= 0;
				*Ascii		= true;
			}
			if(strcmp(ext, "IBA")==0) {
				*FileType	= 0;
				*Ascii		= false;
			}
		} else return false;
		return true;
	}

	void EncodeBasicFile( void ) {
		unsigned char buff[4];
		DWORD NOR, NOW;
		buff[0]=0x0d;
		buff[1]=0x0a;
		WriteFile(hOutFile, buff, 2, &NOW, NULL);
		while(1) {
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) break;
			WriteFile(hOutFile, buff, 1, &NOW, NULL);
		}
		buff[0] = 0x0d;
		buff[1] = 0x0a;
		buff[2] = 0x1a;
		WriteFile(hOutFile, buff, 3, &NOW, NULL);
	}

	// べた書込み
	void EncodeRawFile( void ) {
		unsigned char buff[4];
		DWORD NOR, NOW;
		while(1) {
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) break;
			WriteFile(hOutFile, buff, 1, &NOW, NULL);
		}
	}

	void EncodeDataFile( void ) {
		EncodeBasicFile();			// 同じ処理ですます
	}

	
	bool ReadLine( unsigned char *ptr, int max ) {
		DWORD NOR;
		char buff[4];
		int linecount=0;
		while(1) {
			*ptr = '\0';
			if(linecount>=max) return false;
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) {
				if(linecount==0) return false;
				return true;
			}
			if(buff[0]<' ') break;
			*ptr++ = buff[0];
			linecount++;
		}
		return true;
	}

	void EncodeMachineFile( void )
	{
		unsigned char motbuf[0x10000];			// 64KB確保
		unsigned long minadr, maxadr;
		unsigned char buff[512];
		CMotorolaS mot;
		minadr = 0xffff;
		maxadr = 0x0000;
		unsigned long adr;
		unsigned long byte;
		unsigned long entry;
		while(ReadLine(buff, 510)) {
			if(buff[0]!='S') continue;
			mot.SetSRecord((char*)buff);				// Sレコードをセット
			adr  = mot.GetLoadAddress();				// ロードアドレスを取得
			byte = mot.GetDataCount();					// データ数を取得
			unsigned int i;
			switch(mot.GetRecordType()) {
			case 1:
			case 2:
			case 3:
				if(minadr>adr)			minadr = adr;		// 最小アドレスを記録
				if(maxadr<adr+byte-1u)	maxadr = adr+byte-1;// 最大アドレスを記録
				for(i=0; i<byte; i++) {
					motbuf[adr+i] = mot.GetData(i);			// データをmotrola-sバッファにコピー
//					printf("%02x", motbuf[adr+i]);
				}
//				printf("\n");
//				printf("adr=%04x size=%04x\n", adr, byte);
				break;
			case 7:
			case 8:
			case 9:
				entry = mot.GetLoadAddress();
				break;
			}
		}
//		printf("Min adrs=%04x  Max adrs=%04x  Entry=%04x\n", minadr, maxadr, entry);

		DWORD NOW;
		unsigned long size = maxadr-minadr+1;
		buff[0] = (unsigned char)0x00;
		buff[1] = (unsigned char)(size>>8);
		buff[2] = (unsigned char)(size & 0x00ff);
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// ファイルサイズ書き込み
		buff[0] = (unsigned char)(minadr>>8);
		buff[1] = (unsigned char)(minadr & 0x00ff);
		WriteFile(hOutFile, buff, 2, &NOW, NULL);		// ロードアドレス書き込み
		WriteFile(hOutFile, motbuf+minadr, size, &NOW, NULL);	// 実データ書き込み
		buff[0] = 0xff;
		buff[1] = 0;
		buff[2] = 0;
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// ダミー３バイト書き込み
		buff[0] = (unsigned char)(entry>>8);
		buff[1] = (unsigned char)(entry & 0x00ff);
		buff[2] = 0x1a;			// EOF
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// エントリーアドレス&EOF書き込み
	}

	// ヘッダ情報を表示する
	void ShowHeader( void ) {
		if(fVerbose)
			printf("Filename=%8s FileType=%d Ascii=%c Random=%c\n", filename, FileType, Ascii?'A':'B', Random?'R':'S');
	}

	// ヘッダ情報を書き込む
	void WriteHeader( void )  {
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
		WriteFile(hOutFile, header, 0x10, &NOW, NULL);
	}

public:

	CFMENCODE() {
		fRaw = true;
		fVerbose=false;
	}

	void Usage( void )	{
		puts("DOS/V file to FM-FILE converter " VERSION );
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMENCODE IN_FILE [options]");
		puts("Options: -v  Verbose mode");
		puts("ex. FMENCODE test.txt");
		puts("Note: You can specify the DOS/V file which have following file extention:");
		puts("   txt  : Text file          will convert to '1 A 0' file");
		puts("   mot,s: Motrola-S file     will convert to '2 B 0' file");
		puts("   bas  : Basic file(ASC)    will convert to '0 A 0' file");
		puts("   iba  : Basic file(i-code) will convert to '0 B 0' file");
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
		hInFile = CreateFile(infile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
		if(hInFile==INVALID_HANDLE_VALUE) {
			puts("Failed to open input file");
			exit(1);
		}

		// ベースファイル名取り出し
		GetFileBase(infile, filename);

		// 拡張子取り出し＆解析
		char ext[10];		// ファイル拡張子
		GetFileExtention(infile, ext);
		AnalyzeFileExtention(ext, &FileType, &Ascii, &Random);

		if(fVerbose) 
			printf("fn=%s ext=%s %d %c %c\n", infile, ext, FileType, Ascii==true?'A':'B', Random==true?'1':'0');

		// 出力ファイル名を作成
		sprintf(outfile, "%s.%d%c%d", filename,
			FileType,					// ファイルタイプ 0,1,2
			Ascii?'A':'B',				// アスキーフラグ
			Random?1:0);
		if(fVerbose)
			printf("FM-FILE file ='%s'\n", outfile);

		// 出力ファイルオープン
		hOutFile = CreateFile(outfile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
		if(hOutFile==INVALID_HANDLE_VALUE) {
			puts("Failed to open output file");
			exit(1);
		}

		// ヘッダ情報を書き込む
		WriteHeader();

		switch(FileType) {
		case 0:
			if(Ascii==true) 
				EncodeBasicFile();
			else {
				EncodeRawFile();
			}
			break;
		case 1:
			EncodeDataFile();
			break;
		case 2:
			EncodeMachineFile();
			break;
		case 9:
			// -rオプション指定時
//			DecodeRawFile();
			break;
		default:
			break;
		}
		CloseHandle(hInFile);
		CloseHandle(hOutFile);
	}
};


void main(int argc, char *argv[])
{
	CFMENCODE fmencode;
	fmencode.Ignition(argc, argv);
}
