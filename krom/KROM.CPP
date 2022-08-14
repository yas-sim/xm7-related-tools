#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <conio.h>

#define VERBOSE (0)

FILE *fp;

// JISコードからShiftJISコードに変換する
unsigned short JIS2SJIS( unsigned short c )
{
	int upper, lower;
	upper = (c>>8) & 0x00ff;
	lower = c      & 0x00ff;
	if(upper & 1) {
		if(lower<0x60) {
			lower += 0x1f;
		} else {
			lower += 0x20;
		}
	} else {
		lower += 0x7e;
	}
	if(upper<0x5f) {
		upper = (upper + 0xe1)  >> 1;
	} else {
		upper = (upper + 0x161) >> 1;
	}
	return (upper<<8)+lower;
}

// JISコードで指定された文字を(0,0)からに表示する
void PutChar( HDC hdc, unsigned short jis )
{
	unsigned char cbuff[3];
	unsigned short sjis = JIS2SJIS(jis);
	cbuff[0] = (sjis>>8) & 0x00ff;
	cbuff[1] =  sjis     & 0x00ff;
	cbuff[2] = '\0';
	SetTextAlign(hdc, TA_LEFT | TA_TOP);
	SetTextColor(hdc, RGB(0,0,0));
	SetBkColor(hdc, RGB(255,255,255));
	TextOut(hdc, 0, 0, (LPCTSTR)cbuff, 2);
#if VERBOSE==1
	printf("%s JIS:0x%04x  SJIS:0x%04x\n", cbuff, jis, sjis);
#endif
}

// (0,0)-(15,15)に描かれている文字を読み取り、16進に変換してファイルに書き出す
void ReadChar( HDC hdc )
{
	COLORREF pixel;
	unsigned short v;
#if VERBOSE==1
	printf("++++++++++++++++++\n");
#endif
	for(int y=0; y<16; y++) {
#if VERBOSE==1
		printf("+");
#endif
		v=0;
		for(int x=0; x<16; x++) {
			pixel = GetPixel(hdc, x, y);
			if(pixel & 0x0f) {
				v <<=1;
#if VERBOSE==1
				printf(" ");
#endif
			} else {
				v = (v<<1) | 1;
#if VERBOSE==1
				printf("@");
#endif
			}
		}
#if VERBOSE==1
		printf("+ 0x%04x\n", v);
#endif
		fprintf(fp, "%c%c", ((v>>8) & 0x00ff), v & 0x00ff);
	}
#if VERBOSE==1
	printf("++++++++++++++++++\n");
#endif
}

// 漢字Addres to JIS code convert
// FM-NEW7のユーザーズマニュアルシステム仕様のp.47の表は間違ってるので注意
// 表はJC14,JC13,JC12,JC6,JC5 の順になっているが、JC5,JC6,JC12,JC13,JC14の順の間違い
unsigned short Kad2Jis( unsigned short kad )
{
	const unsigned char tbl[8] = { 0x00, 0x0a, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13 };
	int ra11_9, ra8, tblv;
	unsigned short jis;
	ra11_9 = (kad>>9) & 0x07;
	ra8    = (kad>>8) & 0x01;
	tblv = tbl[ra11_9];
	if(ra11_9==0) {
		if(ra8==0) {
			tblv = 0x09;
		} else {
			tblv = 0x0b;
			ra8=0;
		}
	}
	jis = (((tblv>>2)&0x07)<<12) | ((ra8 & 0x01)<<11) | (((kad>>5)&0x07)<<8) |
			((tblv&0x03)<<5) | (kad & 0x1f);
	return jis;
}

// メインルーチン♪
void main(void)
{
	puts("XM7 KANJI ROM File generator");
	puts("Programmed by an XM7 supporter");
	HDC hDC = CreateDC("DISPLAY", NULL, NULL, NULL);	// 全画面に対するDC取得♪
	if(hDC==INVALID_HANDLE_VALUE) {
		puts("Failed to create device context");
		exit(0);
	}

	fp = fopen("KANJI.ROM", "wb");						// KANJI.ROMファイルオープン
	if(fp==NULL) {
		perror("Failed to create kanji.rom file");
		exit(1);
	}
	puts("Kanji image generating...Wait a moment");
	// 漢字Addressでループさせて全文字コードを取得する
	for(int kad=0; kad<0x1000; kad++) {
		PutChar(hDC, Kad2Jis(kad));
		ReadChar(hDC);
//		while(!_kbhit()); _getch();		// 1文字ごとにキーを押させる場合、コメントを外す
	}
	fclose(fp);
	DeleteDC(hDC);
	puts("Done");
}
