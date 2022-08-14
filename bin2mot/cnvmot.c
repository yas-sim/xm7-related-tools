#include <stdio.h>
#include <stdlib.h>

#include "motgen.h"

FILE *fopen_( char *filename , char *type )
{
	FILE *fp;
	if((fp=fopen(filename, type))==(FILE*)NULL) {
		printf("I couldn't open file '%s' in mode '%s'.\n", filename, type);
		exit(1);
	}
	return fp;
}

void main( int argc , char *argv[] )
{
	FILE *fpi;
	char *ptr;
	int ch;

	if(argc!=2) {
		puts("BINARY TO MOTOROLA-S FORMAT CONVERTER" );
		puts("Programmed by an XM7 supporter");
		printf("\nUsage : %s BIN_FILE\n\n", argv[0]);
		puts("Converted result will be outputed to standard output stream(stdout).");
		exit(-1);
	}

	fpi=fopen_(argv[1], "rb");

	motInit();
	motSetType(motHEADER);			/* ヘッダレコード */
	motSetAddress(0L);
	ptr=argv[1];
	while(*ptr!='\0' /*(char)NULL*/) {		/* ヘッダレコードの中身はファイル名 */
		motStock(*ptr++);
	}
	motFlush();

	motSetType(motDATA24);
	motSetAddress(0L);				/* 次のレコードの開始アドレス */
	while(!feof(fpi)) {
		ch=fgetc(fpi);				/* ファイルから１バイト読み込み */
		if(ch==-1) break;
		motStock((unsigned char)ch);	/* レコードバッファに格納 */
	}
	motFlush();							/* レコードバッファに、未送出データが残っている場合、それを出力する */

	/* 最後にエンドレコードを出力する */
	motSetAddress(0L);
	motSetType(motEND24);			/* エンドレコードタイプ */
	motFlush();

	fclose(fpi);
}
