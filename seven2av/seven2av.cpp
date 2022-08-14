#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION	"v0.3"

// v0.3 -CompatibleROM�I�v�V������ǉ��Bsubsys_a.rom��subsys_b.rom�̎���������}������I�v�V����

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
	
	// SUBSYS_C.ROM�̐擪����0x800�o�C�g��4��R�s�[����SUBSYS_CG.ROM�����
	if((fpo=fopen("SUBSYSCG.ROM", "wb"))==NULL) { fclose(fpi); return false; }
	for(int i=0; i<4; i++) {
		fseek(fpi, 0, SEEK_SET);
		for(int j=0; j<0x800; j++) {
			fputc(fgetc(fpi), fpo);
		}
	}
	fclose(fpo);

	if(fCompatibleROM==false) {
		// SUBSYS_C.ROM��0x800�o�C�g�ڂ���0x2000�o�C�g�����R�s�[����SUBSYS_A.ROM�����
		if((fpo=fopen("SUBSYS_A.ROM", "wb"))==NULL) { fclose(fpi); return false; }
		fseek(fpi, 0x800, SEEK_SET);
		for(int j=0; j<0x2000; j++) {
			fputc(fgetc(fpi), fpo);
		}
		fclose(fpo);

		fclose(fpi);
		// SUBSYS_A.ROM���R�s�[����SUBSYS_B.ROM�����
		return _CopyFile("SUBSYS_A.ROM", "SUBSYS_B.ROM");
	}
	return true;
}

// �C�j�V�G�[�^ROM�����(INITIATE.ROM)
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

	// �u�[�gROM�̕����c����0�Ŗ��߂�
	for(;length<0x1800; length++) fputc(0, fpo);		// �t�@�C���T�C�Y��0x1800�ɂȂ�܂�0�Ŗ��߂�

	// �u�[�gROM(BOOT_BAS.ROM)���R�s�[����
	if((fpi=fopen("BOOT_BAS.ROM", "rb"))==NULL) { fclose(fpi); return false; }
	// ROM�̓��e��S�ăR�s�[����
	for(length=0; length<0x200; length++) {
		fputc(fgetc(fpi), fpo);
	}

	// �u�[�gROM(BOOT_DOS.ROM)���R�s�[����
	if((fpi=fopen("BOOT_DOS.ROM", "rb"))==NULL) { fclose(fpi); return false; }
	// ROM�̓��e��S�ăR�s�[����
	for(length=0; length<0x200; length++) {
		fputc(fgetc(fpi), fpo);
	}

	for(length=0; length<0x0400-2; length++) fputc(0, fpo);	// �t�@�C���̍Ō�-2�܂�0�Ŗ��߂�

	// �C�j�V�G�[�^ROM�֌��������Z�b�g�x�N�^
	fputc(0x60, fpo); fputc(0x00, fpo);

	fclose(fpi);
	fclose(fpo);
	return true;
}

void Title( void ) {
	puts("XM7 File generator for V2 " VERSION);
	puts("���̃v���O�����́AXM7 V1�p�̃t�@�C������AV2�ɕK�v�ȃt�@�C�������o���v���O�����ł�");
	puts("�����f�B���N�g���ɁAV1�p�̈ȉ��̃t�@�C�����K�v�ł�");
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