// XM7�t�H���g�p�b�`�c�[��
#define VERSION "v0.1"

#define FONT_PATCH	(1)
#define BIN_FONT	(2)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// �w��̃t�@�C��������A�g���q�𔲂��o���B�g���q�͑啶���ɕϊ����ĕԂ�
void GetExtension( char *filename, char *buff ) {
	char *ptr = filename;
	while(*filename) {			// ��������Ō�܂Ō���
		if(*filename=='.') ptr = filename+1;	// �Ō�Ɍ������s���I�h�ʒu���L��
		filename++;
	}
	strcpy(buff, ptr);		// �g���q�������R�s�[
	_strupr(buff);			// �啶���ϊ�
}

// SUBSYS_C.ROM�t�@�C���̒��̃t�H���g�Ƀp�b�`�𓖂Ă�
bool FontPatch( FILE *fp, char *fontfile ) {
	// �t�H���g�t�@�C�����I�[�v������
	FILE *fpi;
	if((fpi=fopen(fontfile, "rt"))==NULL) {
		perror("Failed to open font file");
		return false;
	}
	char buff[256];
	int line=-1, ch=0;
	do {
		fgets(buff, 255, fpi);
		if(buff[0]==';' || buff[0]=='#') continue;	// ;��#�Ŏn�܂�s�̓R�����g
		if(line>=0 && ch>0) {		// line�����̐��łȂ����́A�t�H���g�ǂݍ��ݒ�
			int val=0;
			for(int i=0; i<7; i++) {
				val = (val<<1) | (buff[i]=='0'?0:1);
			}
			fseek(fp, ch*8+line, SEEK_SET);	// �t�H���g�ԍ��ƃ��C������t�@�C���|�C���^���ړ�������
			fputc(val, fp);
			if(++line>7) line=-1;			// 8���C��(1�t�H���g)���ǂݏI������̂ŁA�ҋ@��Ԃɖ߂�
		}
		if(buff[0]=='@') {	// �t�H���g�ԍ��w��
			line=0;			// �t�H���g�ǂݍ��ݒ��ɂ���
			sscanf(buff+1, "%x", &ch);		// �t�H���g�ԍ�(16�i)��ǂݍ���
		}
	} while(!feof(fpi));
	rewind(fp);			// SUBSYS_C.ROM�t�@�C���̃t�@�C���|�C���^��擪�ɂ܂��߂�
	fclose(fpi);
	return true;
}

// SUBSYS_C.ROM�t�@�C���̐擪����ɁA�w��̃o�C�i���t�H���g�t�@�C���̒��g���㏑������
bool BinFont( FILE *fp, char *fontfile ) {
	// �t�H���g�t�@�C�����I�[�v������
	FILE *fpi;
	if((fpi=fopen(fontfile, "rb"))==NULL) {
		perror("Failed to open font file");
		return false;
	}
	rewind(fp);			// SUBSYS_C.ROM�t�@�C���̃t�@�C���|�C���^��擪�ɂ܂��߂�
	int ch;
	while((ch=fgetc(fpi))>=0) {	// �o�C�i���t�H���g�̒��g��SUBSYS_C.ROM�̐擪����ɏ㏑������
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
	puts("fontfile�̊g���q��'txt'�̂Ƃ��́A�t�H���g�p�b�`�`��");
	puts("    �V            'bin'�̂Ƃ��́A�o�C�i���t�H���g�`��");
	puts("�Ƃ���ROM file(SUBSYS_C.ROM�܂���SUBSYSCG.ROM)�̒��g������������");
	puts("SUBSYSCG.ROM��XM7 V2�p�̃t�@�C���ŁASUBSYS_C.ROM��XM7 V1�p�̃t�@�C���ł�");
	puts("���̃v���O�����́A���SUBSYSCG.ROM���I�[�v�����A���߂Ȃ�SUBSYS_C.ROM���I�[�v�����悤�Ƃ��܂�");
	puts("SUBSYS_C.ROM�܂��́ASUBSYSCG.ROM�t�@�C���́A���̃t�@�C���Ɠ����f�B���N�g���ɂ����Ă����K�v������܂�");
}

void main( int argc, char *argv[] ) {
	int nFileType = 0;

	title();

	if(argc!=2) {
		usage();
		exit(1);
	}

	// �g���q����t�@�C���`���𔻒f����
	char extension[10];
	GetExtension(argv[1], extension);
	if(stricmp(extension, "txt")==0) nFileType = FONT_PATCH;	// �t�H���g�p�b�`�`��
	if(stricmp(extension, "bin")==0) nFileType = BIN_FONT;		// �o�C�i���t�H���g�`��
	if(nFileType==0) {		// txt, bin�ȊO�̃t�@�C���`�����w�肵���ꍇ
		puts("Unsupported file specified");
		usage();
		exit(1);
	}

	// ���SUBSYS_CG.ROM(V2�p)���J���A���߂Ȃ�SUBSYS_C.ROM(V1�p)���J��
	FILE *fpo;
	if((fpo=fopen("SUBSYSCG.ROM", "rb+"))==NULL) {	// �㏑�����[�h�Ńt�@�C���I�[�v��		
		if((fpo=fopen("SUBSYS_C.ROM", "rb+"))==NULL) {
			perror("Failed to open 'SUBSYS_C.ROM(V1) or SUBSYS_CG.ROM(V2)' file");
			exit(1);
		} else {
			puts("Target file is the SUBSYS_C.ROM");
		}
	} else {
			puts("Target file is the SUBSYSCG.ROM");
	} 

	// �t�@�C���`���ʂɏ������s��
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

// v0.0	���񃊃��[�X
// v0.1	000521	SUBSYS_CG.ROM���ɃI�[�v�����悤�Ƃ���悤�ɕύX(V2�Ή�)
