// モトローラSファイルを入力し、そのSレコードを全て展開したバイナリイメージファイルを作成する。
// また、オプション指定することにより、任意の個数のROM, ROMデータ幅に対応したSレコードファイルを
// 生成する。生成されたファイルをそれぞれのROMに書き込むことで、CPUからは元のSレコードファイルの
// イメージが見える。

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <process.h>

#define _ALLOC_UNIT		(0x1000)		// メモリイメージバッファを割り当てるときの割り当て単位
#define MOTS_ADRMASK	(~0xe0000000)

#include "cmotorolas.h"
#include "crange.h"

unsigned char *pBuff;					// メモリイメージバッファへのポインタ
unsigned long nBufSiz;					// 現在のバッファサイズ（可変）
unsigned long nMaxAddr;					// 現在までのイメージバッファ内の最高アドレス(書き出し時に使用)

CRanges oRanges;						// 入力Sレコードファイルに存在していた範囲を記録する(Sレコード出力時に、データのある部分だけを出力するのに使用)

bool fVerbose;							// さまざまなメッセージを表示させるかどうか

// メモリイメージバッファを指定のサイズで確保する。
// メモリ割り当て単位に切り上げて確保する
bool BuffAlloc( unsigned long size )
{
	if(nBufSiz<size) {
		unsigned long nAllocSize = ((size+_ALLOC_UNIT-1)/_ALLOC_UNIT)*_ALLOC_UNIT;		// 割り当てサイズ丸め込み
		unsigned char *pTmp = (unsigned char*)realloc(pBuff, nAllocSize);				// メモリ確保！
		if(pTmp==NULL) {						// 割り当て失敗
			puts("Memory allocation failed.");
			puts("Too few memory");
			exit(-1);
		} else {								// 割り当て成功
			pBuff = pTmp;
			nBufSiz = nAllocSize;
		}
		if(fVerbose) printf("BuffMemAlloc: %08x\n", nAllocSize);
	}
	return true;
}


// 与えられた文字列をSレコードとみなし、解析する
// pBuffにそのSレコードのデータを書き込む
void mot2bin( char *pLine )
{
	CMotorolaS oMot;
	int ptr = 0;
	unsigned long addr;
	int length;

	oMot.SetSRecord(pLine);					// CMotorolaSオブジェクトにファイルから読み込んだ行を渡す
	addr = oMot.GetLoadAddress();			// 解析結果のロードアドレスを取得
	length = oMot.GetDataCount();			// データバイト数を取得
	addr = addr & MOTS_ADRMASK;				// SHは上位アドレスは出力されないのでマスクする

	oRanges.add(addr, addr+length-1);		// 使用範囲を記録する

	// 1行分のデータをバッファに書き込む
	int ch, type;
	for(int i=0; i<length; i++) {
		ch = oMot.GetData(i);					// データから１バイト読み込む
		type = oMot.GetRecordType();
		if(type==1 || type==2 || type==3) {		// データレコードのときだけ処理を行う。ヘッダレコードなどは無視する
			while(addr>=nBufSiz) {					// 書き込むアドレスがバッファ容量を越えていなかチェック
				BuffAlloc(addr+1);					// バッファ容量を割増して再割り当て
			}
			pBuff[addr] = ch;
			if(nMaxAddr<addr) {						// 最大書込みアドレスを記録する
				nMaxAddr = addr;
			}
			pLine += 2;								// アスキー化16進を扱うので、１バイト処理ごとに２バイトポインタを進める
			addr ++;
		}
	}
}

// CPUから見たアドレスから、ROMから見たアドレスに変換する。
// ROMの個数、ROM1個のビット幅などからROM番号、ROMアドレス、バイトアドレスを計算する
// 本当にあっているかちょっと不安
void CalcRomAddress( unsigned long nAddress,	// CPUから見たアドレス
					int nRoms,					// ROMの個数
					int nRomWidth,				// ROM1個のバイト数(16ビット幅なら2)
					int &nRomNumber,			// 該当するROM番号
					unsigned long &nRomAddress,	// ROMから見たアドレス(バス幅に関係ない、真のアドレス。ROMのアドレスバスの値ではない)
					int &nByteAddress)			// ROM内のバイトアドレス

// |        Address                        |
// |RomAddressHIGH|XXXXXXXXXX|RomAddressLOW| ROMアドレスはXXXXの部分だけ削って、HIGHの部分は左にシフトさせる
// |              |nRomNumber|nByteAddress |
{
	int i;
	for(i=0; !(nRoms     & 1); i++) nRoms    >>=1; nRoms = i;		// nRoms = log(nRoms)/log(2)
	for(i=0; !(nRomWidth & 1); i++) nRomWidth>>=1; nRomWidth = i;	// nRomWidth = log(nRomWidth)/log(2)

	// 各データを割り出すためのビットマスクなどを生成
	unsigned long BAmask, RNmask, RAmask, ByteShift;
	BAmask = (1<<(nRomWidth))-1;					// バイトアドレス用のマスク
	RAmask = ~BAmask;								// ROMアドレス用のマスク
	RNmask = ((1<<(nRomWidth+nRoms))-1) & RAmask;	// ROM番号用のマスク
	ByteShift = nRomWidth;
	
	nByteAddress = nAddress & BAmask;
	nRomAddress = ((nAddress>>nRoms) & RAmask) | nByteAddress;
	nRomNumber = (nAddress & RNmask) >> ByteShift;
}

// 作成したメモリイメージバッファの中身を、指定のROM個数、ROMデータ幅として複数の
// MOTファイルに分割して出力する
void motout(int nRoms, int nRomWidthInByte, CRanges &ranges)
{
	int nRWIB = nRomWidthInByte;
	if(nRoms==0) return;			// ROMの個数が0はNG
	if(nRWIB==0) return;			// ROMのビット幅が0はNG

	// 指定のROM個数分ファイルを開く。出力ファイル名はROMx.motに固定
	FILE **fpo = new FILE*[nRoms];		// ファイルハンドル用ポインタを確保
	char filename[32];
	for(int i=0; i<nRoms; i++) {
		sprintf(filename, "ROM%d.mot", i);
		fpo[i] = fopen(filename, "wt");
	}

	CMotorolaS *mot = new CMotorolaS[nRoms];		// ROMの個数分CMotorolaSオブジェクトを確保
	CRange range;
	int RN, BA;										// RN=RomNumber, BA=ByteAddress
	unsigned long RA;								// RA=RomAddress
	char tmp[256];									// Sレコード文字列を一時的に格納するためのバッファ

	// Sレコード解析時に登録しておいた使用範囲全てを出力する
	for(i=0; i<ranges.getNumberOfItems(); i++) {
		ranges.getRange(i, range);					// i番目の範囲情報を取得
		for(int j=0; j<nRoms; j++) {				// ROMの個数分CMotorolaSオブジェクトを初期化
			mot[j].SetDataCount(0);					// MotorolaSオブジェクトのデータサイズを0に初期化
		}
		// 取得した範囲の間のデータを全て出力する
		for(unsigned long adr=range.top; adr<=range.bottom; adr++) {
			CalcRomAddress(adr, nRoms, nRomWidthInByte, RN, RA, BA);		// ROMアドレスを計算
			if(mot[RN].GetDataCount()==0) {									// CMotorolaSレコードの保持データ数が0のときは初期化しただけなので、ロードアドレスなどを設定する必要がある
				mot[RN].InitSRecord(RA);									// ロードアドレス、レコードタイプを設定する
			}
			mot[RN].SetData(RA-mot[RN].GetLoadAddress(), pBuff[adr]);		// Sオブジェクトの指定の位置にデータを追加
			if(mot[RN].GetDataCount()>=0x10) {								// Sオブジェクトの保持データ数が0x10になったらファイルに出力し、保持データをクリアする
				fputs(mot[RN].GetSRecordBuff(tmp), fpo[RN]);				// Sレコード文字列を出力
				fputs("\n", fpo[RN]);
				mot[RN].SetDataCount(0);									// 保持データ数を0にする
			}
		}
		// ファイルに出力されずに残ってしまっているSレコード情報をファイルに書き出す（フラッシュ）
		for(j=0; j<nRoms; j++) {
			if(mot[j].GetDataCount()>0) {						// 残ってるか？
				fputs(mot[j].GetSRecordBuff(tmp), fpo[j]);		// Sレコード文字列を出力
				fputs("\n", fpo[j]);
				mot[j].SetDataCount(0);
			}
		}
	}
	// 全てのファイルハンドルをクローズ
	for(i=0; i<nRoms; i++) {
		fclose(fpo[i]);
	}
	delete []fpo;	// newで確保したオブジェクトは開放する
	delete []mot;	// newで確保したオブジェクトは開放する
}

// 使い方の表示
void usage( void )
{
	puts("");
	puts("Synopsis: mot2bin [options] inputfile [outputfile]");
	puts("Options:   -r : A number of ROMs(1, 2, 4, 8, ...)");
	puts("           -w : ROM data width in byte (1, 2, 4, 8, ...)");
	puts("           -s : Output splited ROM image file(ROMx.mot)");
	puts("           -v : Verbose mode");
	puts("           -h : Help");
	puts("Note:  1. The outputfile will assume to be 'temp.bin', if you omit the outputfile.");
	puts("ie:  C:>mot2bin -s -r2 -w1 check.mot check.bin");
	puts("     ROM 2pcs, 16bit width ROM");
}

// 与えられた文字列を数値に変換して、その値が2^xの値かどうかチェックする
// 2^x以外の値だった場合引数rtnを返す。2^xだったら、変換した数値を返す
// main()の引数評価用のカスタム関数
int CheckPow2( char *str, int rtn )
{
	int tmp1, tmp2;
	tmp1 = atoi(str);						// 引数を数値に変換
	for(tmp2=1; tmp2<=0x0020; tmp2<<=1) {	// 引数が2^xの値になっているかチェック
		if(tmp2==tmp1) return tmp1;
	}
	return rtn;
}

void main( int argc, char *argv[])
{
	FILE *fo;
	FILE *fi;

	char sourcefile[128], outputfile[128];		// 入力ファイル名

	puts("*** Motorola-S to Binary converter ***");
	puts("Copyright 1999 EASTON Co.,Ltd. All rights reserved.");

	// オプション解析
	int nNumberOfRoms = 1;			// ROMの個数（分割数）
	int nRomWidthInByte = 1;		// ROM１個のバス幅（バイト単位）
	sourcefile[0] = '\0';
	strcpy(outputfile, "temp.bin");	// 出力ファイル省略時のファイル名
	bool fExit, fSplit;
	fExit = false;
	fSplit = false;
	fVerbose = false;
	for(int i=1; i<argc; i++) {
		if(argv[i][0]=='-' || argv[i][0]=='/') {		// 引数の１文字目が'-'か'/'ならオプション指定とみなす
			switch(argv[i][1]) {
			case 'r':
			case 'R':		// ROMの個数
				nNumberOfRoms = CheckPow2(argv[i]+2, nNumberOfRoms);
				break;
			case 'w':
			case 'W':		// ROM１個あたりのバス幅（バイト単位）
				nRomWidthInByte = CheckPow2(argv[i]+2, nRomWidthInByte);
				break;
			case 'v':
			case 'V':
				fVerbose = true;
				break;
			case 's':
			case 'S':
				fSplit = true;
				break;
			case 'h':
			case 'H':
			case '?':
				fExit = true;
				break;
			default:
				printf("Illegal option '%s'\n", argv[i]);
				fExit = true;
				break;
			}
		} else {
			if(strlen(sourcefile)==0) {
				strncpy(sourcefile, argv[i], 127);
			} else {
				strncpy(outputfile, argv[i], 127);
			}
		}
	}
	if(strlen(sourcefile)==0) {
		printf("Specify source file name\n");
		fExit = true;
	}
	if(strlen(outputfile)==0) {
		printf("Specify output file name\n");
		fExit = true;
	}
	if(fExit) {
		usage();
		exit(-1);								// オプション解析で問題があったとき（ヘルプ表示のときも）	
	}

	// 入力ファイルをオープン
	if((fi = fopen(sourcefile, "rt"))==NULL) {
		puts("Couldn't open source file.");
		puts(argv[1]);
		exit(-1);
	}
	
	// 出力ファイルをオープン
	if((fo = fopen(outputfile, "wb"))==NULL) {
		puts("Couldn't open output file.");
		puts(argv[2]);
		fcloseall();
		exit(-1);
	}

	// 条件表示
	if(fVerbose) {
		printf("*** ROM=%dpcs, ROM DATA BUS WIDTH=%dbit ***\n", nNumberOfRoms, nRomWidthInByte*8);
		printf("INPUT %s  OUTPUT %s\n", sourcefile, outputfile);
	}

	// 初期メモリイメージバッファ割り当て
	pBuff = (unsigned char*)calloc(_ALLOC_UNIT, 1);
	if(pBuff==NULL) {
		puts("Too few memory");
		fcloseall();
		exit(-1);
	}
	nBufSiz = _ALLOC_UNIT;
	nMaxAddr = 0L;

	// 入力ファイル読み込み
	char line[512];
	while(1) {
		fgets(line, 511, fi);			// １行読み込み
		if(feof(fi)) break;
		mot2bin(line);					// Sレコードをメモリイメージバッファに書き込む
	}

	fwrite(pBuff, 1, nMaxAddr+1, fo);	// メモリイメージファイル書き出し

	oRanges.combineAll();				// 記録した使用範囲のうち、結合できる範囲を結合する
	
	if(fSplit) motout(nNumberOfRoms, nRomWidthInByte, oRanges);		// モトローラSフォーマットで出力

	// 入力ファイルの中で使用した領域を表示
	if(fVerbose) {
		puts("*** Used area");
		oRanges.show();					// 使用範囲一覧を表示する
	}
	if(fVerbose) printf("*** Max address = 0x%08x\n", nMaxAddr);	// 使用した最大アドレスを表示
	printf("*** File conversion succeeded.");
	if(fVerbose) puts(" Trust me!"); else puts("");

	free(pBuff);
	fcloseall();
}
