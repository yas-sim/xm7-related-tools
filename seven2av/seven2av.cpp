#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION	"v0.3"

// v0.3 -CompatibleROMオプションを追加。subsys_a.romとsubsys_b.romの自動生成を抑制するオプション

bool _CopyFile( char *src, char *dst ) {
	FILE *fpi, *fpo;
	if((fpi=fopen(src, "rb"))==NULL) return false;
	if((fpo=fopen(dst, "wb"))==NULL) {	fclose(fpi); return false; }
	int data;
	while(data=fgetc(fpi)>=0) {
		fputc(data,fpo);
	}
	fclose(fpi);
	fclose(fpo);
	return true;
}


// Process SUBSYS_C.ROM file
bool SUBSYS( bool fCompatibleROM ) {
	FILE *fpi, *fpo;
	if((fpi=fopen("SUBSYS_C.ROM", "rb"))==NULL) return false;
	
	// SUBSYS_C.ROMの先頭から0x800バイトを4回コピーしてSUBSYS_CG.ROMを作る
	if((fpo=fopen("SUBSYSCG.ROM", "wb"))==NULL) { fclose(fpi); return false; }
	for(int i=0; i<4; i++) {
		fseek(fpi, 0, SEEK_SET);
		for(int j=0; j<0x800; j++) {
			fputc(fgetc(fpi), fpo);
		}
	}
	fclose(fpo);

	if(fCompatibleROM==false) {
		// SUBSYS_C.ROMの0x800バイト目から0x2000バイト分をコピーしてSUBSYS_A.ROMを作る
		if((fpo=fopen("SUBSYS_A.ROM", "wb"))==NULL) { fclose(fpi); return false; }
		fseek(fpi, 0x800, SEEK_SET);
		for(int j=0; j<0x2000; j++) {
			fputc(fgetc(fpi), fpo);
		}
		fclose(fpo);

		fclose(fpi);
		// SUBSYS_A.ROMをコピーしてSUBSYS_B.ROMを作る
		return _CopyFile("SUBSYS_A.ROM", "SUBSYS_B.ROM");
	}
	return true;
}

// イニシエータROMを作る(INITIATE.ROM)
bool INITIATE( void ) {
	FILE *fpi, *fpo;
	if((fpo=fopen("INITIATE.ROM", "wb"))==NULL) return false;
	if((fpi=fopen("INITIATE.BIN", "rb"))==NULL) { fclose(fpi); return false; }

	int length = 0;
	int data;
	while((data=fgetc(fpi))>=0) {
		length++;
		fputc(data,fpo);
	}
	fclose(fpi);

	// ブートROMの分を残して0で埋める
	for(;length<0x1800; length++) fputc(0, fpo);		// ファイルサイズが0x1800になるまで0で埋める

	// ブートROM(BOOT_BAS.ROM)をコピーする
	if((fpi=fopen("BOOT_BAS.ROM", "rb"))==NULL) { fclose(fpi); return false; }
	// ROMの内容を全てコピーする
	for(length=0; length<0x200; length++) {
		fputc(fgetc(fpi), fpo);
	}

	// ブートROM(BOOT_DOS.ROM)をコピーする
	if((fpi=fopen("BOOT_DOS.ROM", "rb"))==NULL) { fclose(fpi); return false; }
	// ROMの内容を全てコピーする
	for(length=0; length<0x200; length++) {
		fputc(fgetc(fpi), fpo);
	}

	for(length=0; length<0x0400-2; length++) fputc(0, fpo);	// ファイルの最後-2まで0で埋める

	// イニシエータROMへ向けたリセットベクタ
	fputc(0x60, fpo); fputc(0x00, fpo);

	fclose(fpi);
	fclose(fpo);
	return true;
}

void Title( void ) {
	puts("XM7 File generator for V2 " VERSION);
	puts("このプログラムは、XM7 V1用のファイルから、V2に必要なファイルを作り出すプログラムです");
	puts("同じディレクトリに、V1用の以下のファイルが必要です");
	puts("BOOT_BAS.ROM, BOOT_DOS.ROM, SUBSYS_C.ROM");
}

void main( int argc, char *argv[] )
{
	bool fCompatibleROM = false;
	if(argc==2) {
		if(stricmp(argv[1], "-compatiblerom")==0) {
			fCompatibleROM = true;
			puts("Generate compatible ROM mode");
		}
	}
	Title();
	if(!SUBSYS(fCompatibleROM))	{ puts("Error!"); exit(1); }
	if(!INITIATE())	{ puts("Error!"); exit(1); }
	puts("Succeed to generate V2 files!"); 
}