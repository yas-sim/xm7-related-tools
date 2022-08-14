#include <stdio.h>
#include <stdlib.h>

#define VERSION		"V0.2"

// v0.2 プログラム終了時のリターンコードがおかしかったのを修正

void Usage( void )
{
	puts("Usage: bincut infile outfile start_adr end_adr");
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		Usage();
		exit(-1);
	}
	puts("Binary cutter " VERSION);
	puts("Programmed by an XM7 supporter");

	FILE *fpi, *fpo;

	if((fpi=fopen(argv[1], "rb"))==NULL) {
		perror("Input file open");
	}
	if((fpo=fopen(argv[2], "wb"))==NULL) {
		perror("Output file open");
	}
	unsigned long start, end;
	sscanf(argv[3], "%lx", &start);
	sscanf(argv[4], "%lx", &end);

	if(start>end) {
		printf("The end address specified is lower than start address.(ST=%lx, END=%lx)\n", start, end);
		exit(-1);
	}
	if(fseek(fpi, start, SEEK_SET)) {
		perror("File pointer seek error(The input file may be shorter than you expected)");
		exit(-1);
	}
	for(unsigned long i=0; i<=end-start; i++) {
		int ch = fgetc(fpi);
		if(ch==-1) {
			puts("Reached to file end.");
			exit(-2);
		}
		fputc(ch, fpo);
	}
	fcloseall();
	return 0;
}
