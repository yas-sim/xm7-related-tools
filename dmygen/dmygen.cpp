#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	if(argc<3) return -1;
	if((fp=fopen(argv[1], "wb"))==NULL) {
		perror("");
		return -1;
	}
	long size;
	sscanf(argv[2], "%lx", &size);
	printf("Genarating 0x%lx bytes of dummy file '%s'.\n", size, argv[1]);
	for(int i=0; i<size; i++) {
		fputc(0, fp);
	}
	fclose(fp);
	return 0;	
}
