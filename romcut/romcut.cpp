// ROMライタから吸い出したROMファイルからXM7で利用するファイルを切り出すプログラム
#include <stdio.h>
#include <stdlib.h>

// v0.1 000318 表示メッセージを修正(機能変更なし)

#define VERSION "v0.1"

// srcのstartからend番地までをdstファイルに書き出す。
bool cut( const char *src, const char *dst, const long start, const long end ) {
	printf("$%04lx to $%04lx of '%s' is cut for '%s'\n", start, end, src, dst); 
	FILE *fi, *fo;
	fi = fopen(src, "rb");
	if(fi==NULL) {
		printf("Failed to open source file '%s'\n", src);
		return false;
	}
	fo = fopen(dst, "wb");
	if(fo==NULL) {
		printf("Failed to create output file '%s'\n", dst);
		fclose(fi);
		return false;
	}
	for(int skip=0; skip<start; skip++) {
		fgetc(fi);
		if(feof(fi)) {
			printf("Unexpected EOF in '%s'\n", src);
			fclose(fi);
			fclose(fo);
			return false;
		}
	}
	int ch;
	for(int adr=start; adr<=end; adr++) {
		ch = fgetc(fi);
		if(ch<0 || feof(fi)) {
			printf("Unexpected EOF in '%s'\n", src);
			fclose(fi);
			fclose(fo);
			return false;
		}
		fputc(ch, fo);
	}
	fclose(fi);
	fclose(fo);
	return true;
}

#define ABORT(formula)	if((formula)==false) { puts("Program aborted"); exit(1); }

int main( void ) {
	puts("XM7 ROM CUTTER " VERSION);
	puts("Programmed by an XM7 supporter");
	puts("");
	puts("This program requires the following files:");
	puts(" 1. FBASIC.BIN");
	puts(" 2. SUBMON.BIN");
	puts(" 3. BOOTROM.BIN");
	puts("You have to extract above files from your FM-7 ROM before you run this program.\n");

	ABORT(cut("FBASIC.BIN", "FBASIC30.ROM", 0x0000, 0x7bff));
	ABORT(cut("SUBMON.BIN", "SUBSYS_C.ROM", 0x5800, 0x7fff));
	ABORT(cut("BOOTROM.BIN", "BOOT_BAS.ROM", 0x0000, 0x01ff));
	ABORT(cut("BOOTROM.BIN", "BOOT_DOS.ROM", 0x0400, 0x05ff));
	
	return 0;
}
