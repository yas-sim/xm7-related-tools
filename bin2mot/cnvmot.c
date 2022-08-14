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
	motSetType(motHEADER);			/* �w�b�_���R�[�h */
	motSetAddress(0L);
	ptr=argv[1];
	while(*ptr!=(char)NULL) {		/* �w�b�_���R�[�h�̒��g�̓t�@�C���� */
		motStock(*ptr++);
	}
	motFlush();

	motSetType(motDATA24);
	motSetAddress(0L);				/* ���̃��R�[�h�̊J�n�A�h���X */
	while(!feof(fpi)) {
		ch=fgetc(fpi);				/* �t�@�C������P�o�C�g�ǂݍ��� */
		if(ch==-1) break;
		motStock((unsigned char)ch);	/* ���R�[�h�o�b�t�@�Ɋi�[ */
	}
	motFlush();							/* ���R�[�h�o�b�t�@�ɁA�����o�f�[�^���c���Ă���ꍇ�A������o�͂��� */

	/* �Ō�ɃG���h���R�[�h���o�͂��� */
	motSetAddress(0L);
	motSetType(motEND24);			/* �G���h���R�[�h�^�C�v */
	motFlush();

	fclose(fpi);
}
