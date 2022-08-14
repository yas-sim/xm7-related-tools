// XM7フォントパッチツール
#define VERSION "v0.1"

#define FONT_PATCH	(1)
#define BIN_FONT	(2)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 指定のファイル名から、拡張子を抜き出す。拡張子は大文字に変換して返す
void GetExtension( char *filename, char *buff ) {
	char *ptr = filename;
	while(*filename) {			// 文字列を最後まで検索
		if(*filename=='.') ptr = filename+1;	// 最後に見つけたピリオド位置を記憶
		filename++;
	}
	strcpy(buff, ptr);		// 拡張子部分をコピー
	_strupr(buff);			// 大文字変換
}

// SUBSYS_C.ROMファイルの中のフォントにパッチを当てる
bool FontPatch( FILE *fp, char *fontfile ) {
	// フォントファイルをオープンする
	FILE *fpi;
	if((fpi=fopen(fontfile, "rt"))==NULL) {
		perror("Failed to open font file");
		return false;
	}
	char buff[256];
	int line=-1, ch=0;
	do {
		fgets(buff, 255, fpi);
		if(buff[0]==';' || buff[0]=='#') continue;	// ;と#で始まる行はコメント
		if(line>=0 && ch>0) {		// lineが負の数でない時は、フォント読み込み中
			int val=0;
			for(int i=0; i<7; i++) {
				val = (val<<1) | (buff[i]=='0'?0:1);
			}
			fseek(fp, ch*8+line, SEEK_SET);	// フォント番号とラインからファイルポインタを移動させる
			fputc(val, fp);
			if(++line>7) line=-1;			// 8ライン(1フォント)分読み終わったので、待機状態に戻す
		}
		if(buff[0]=='@') {	// フォント番号指定
			line=0;			// フォント読み込み中にする
			sscanf(buff+1, "%x", &ch);		// フォント番号(16進)を読み込む
		}
	} while(!feof(fpi));
	rewind(fp);			// SUBSYS_C.ROMファイルのファイルポインタを先頭にまき戻す
	fclose(fpi);
	return true;
}

// SUBSYS_C.ROMファイルの先頭からに、指定のバイナリフォントファイルの中身を上書きする
bool BinFont( FILE *fp, char *fontfile ) {
	// フォントファイルをオープンする
	FILE *fpi;
	if((fpi=fopen(fontfile, "rb"))==NULL) {
		perror("Failed to open font file");
		return false;
	}
	rewind(fp);			// SUBSYS_C.ROMファイルのファイルポインタを先頭にまき戻す
	int ch;
	while((ch=fgetc(fpi))>=0) {	// バイナリフォントの中身をSUBSYS_C.ROMの先頭からに上書きする
		fputc(ch, fp);
	}
	fclose(fpi);
	return true;
}

void title( void ) {
	puts("XM7(V1) Font patch tool " VERSION);
	puts("Programmed by an XM7 supporter");
}

void usage( void ) {
	puts("SYNOPSIS: fontp fontfile");
	puts("fontfileの拡張子が'txt'のときは、フォントパッチ形式");
	puts("    〃            'bin'のときは、バイナリフォント形式");
	puts("としてROM file(SUBSYS_C.ROMまたはSUBSYSCG.ROM)の中身を書き換える");
	puts("SUBSYSCG.ROMはXM7 V2用のファイルで、SUBSYS_C.ROMはXM7 V1用のファイルです");
	puts("このプログラムは、先にSUBSYSCG.ROMをオープンし、だめならSUBSYS_C.ROMをオープンしようとします");
	puts("SUBSYS_C.ROMまたは、SUBSYSCG.ROMファイルは、このファイルと同じディレクトリにおいておく必要があります");
}

void main( int argc, char *argv[] ) {
	int nFileType = 0;

	title();

	if(argc!=2) {
		usage();
		exit(1);
	}

	// 拡張子からファイル形式を判断する
	char extension[10];
	GetExtension(argv[1], extension);
	if(stricmp(extension, "txt")==0) nFileType = FONT_PATCH;	// フォントパッチ形式
	if(stricmp(extension, "bin")==0) nFileType = BIN_FONT;		// バイナリフォント形式
	if(nFileType==0) {		// txt, bin以外のファイル形式を指定した場合
		puts("Unsupported file specified");
		usage();
		exit(1);
	}

	// 先にSUBSYS_CG.ROM(V2用)を開き、だめならSUBSYS_C.ROM(V1用)を開く
	FILE *fpo;
	if((fpo=fopen("SUBSYSCG.ROM", "rb+"))==NULL) {	// 上書きモードでファイルオープン		
		if((fpo=fopen("SUBSYS_C.ROM", "rb+"))==NULL) {
			perror("Failed to open 'SUBSYS_C.ROM(V1) or SUBSYS_CG.ROM(V2)' file");
			exit(1);
		} else {
			puts("Target file is the SUBSYS_C.ROM");
		}
	} else {
			puts("Target file is the SUBSYSCG.ROM");
	} 

	// ファイル形式別に処理を行う
	switch(nFileType) {
	case FONT_PATCH:
		FontPatch(fpo, argv[1]);
		break;
	case BIN_FONT:
		BinFont(fpo, argv[1]);
		break;
	}
	
	puts("Done!");
	fclose(fpo);
}

// v0.0	初回リリース
// v0.1	000521	SUBSYS_CG.ROMを先にオープンしようとするように変更(V2対応)
