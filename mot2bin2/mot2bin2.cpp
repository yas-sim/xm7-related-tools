// ���g���[��S�t�@�C������͂��A����S���R�[�h��S�ēW�J�����o�C�i���C���[�W�t�@�C�����쐬����B
// �܂��A�I�v�V�����w�肷�邱�Ƃɂ��A�C�ӂ̌���ROM, ROM�f�[�^���ɑΉ�����S���R�[�h�t�@�C����
// ��������B�������ꂽ�t�@�C�������ꂼ���ROM�ɏ������ނ��ƂŁACPU����͌���S���R�[�h�t�@�C����
// �C���[�W��������B

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <process.h>

#define _ALLOC_UNIT		(0x1000)		// �������C���[�W�o�b�t�@�����蓖�Ă�Ƃ��̊��蓖�ĒP��
#define MOTS_ADRMASK	(~0xe0000000)

#include "cmotorolas.h"
#include "crange.h"

unsigned char *pBuff;					// �������C���[�W�o�b�t�@�ւ̃|�C���^
unsigned long nBufSiz;					// ���݂̃o�b�t�@�T�C�Y�i�ρj
unsigned long nMaxAddr;					// ���݂܂ł̃C���[�W�o�b�t�@���̍ō��A�h���X(�����o�����Ɏg�p)

CRanges oRanges;						// ����S���R�[�h�t�@�C���ɑ��݂��Ă����͈͂��L�^����(S���R�[�h�o�͎��ɁA�f�[�^�̂��镔���������o�͂���̂Ɏg�p)

bool fVerbose;							// ���܂��܂ȃ��b�Z�[�W��\�������邩�ǂ���

// �������C���[�W�o�b�t�@���w��̃T�C�Y�Ŋm�ۂ���B
// ���������蓖�ĒP�ʂɐ؂�グ�Ċm�ۂ���
bool BuffAlloc( unsigned long size )
{
	if(nBufSiz<size) {
		unsigned long nAllocSize = ((size+_ALLOC_UNIT-1)/_ALLOC_UNIT)*_ALLOC_UNIT;		// ���蓖�ăT�C�Y�ۂߍ���
		unsigned char *pTmp = (unsigned char*)realloc(pBuff, nAllocSize);				// �������m�ہI
		if(pTmp==NULL) {						// ���蓖�Ď��s
			puts("Memory allocation failed.");
			puts("Too few memory");
			exit(-1);
		} else {								// ���蓖�Đ���
			pBuff = pTmp;
			nBufSiz = nAllocSize;
		}
		if(fVerbose) printf("BuffMemAlloc: %08x\n", nAllocSize);
	}
	return true;
}


// �^����ꂽ�������S���R�[�h�Ƃ݂Ȃ��A��͂���
// pBuff�ɂ���S���R�[�h�̃f�[�^����������
void mot2bin( char *pLine )
{
	CMotorolaS oMot;
	int ptr = 0;
	unsigned long addr;
	int length;

	oMot.SetSRecord(pLine);					// CMotorolaS�I�u�W�F�N�g�Ƀt�@�C������ǂݍ��񂾍s��n��
	addr = oMot.GetLoadAddress();			// ��͌��ʂ̃��[�h�A�h���X���擾
	length = oMot.GetDataCount();			// �f�[�^�o�C�g�����擾
	addr = addr & MOTS_ADRMASK;				// SH�͏�ʃA�h���X�͏o�͂���Ȃ��̂Ń}�X�N����

	oRanges.add(addr, addr+length-1);		// �g�p�͈͂��L�^����

	// 1�s���̃f�[�^���o�b�t�@�ɏ�������
	int ch, type;
	for(int i=0; i<length; i++) {
		ch = oMot.GetData(i);					// �f�[�^����P�o�C�g�ǂݍ���
		type = oMot.GetRecordType();
		if(type==1 || type==2 || type==3) {		// �f�[�^���R�[�h�̂Ƃ������������s���B�w�b�_���R�[�h�Ȃǂ͖�������
			while(addr>=nBufSiz) {					// �������ރA�h���X���o�b�t�@�e�ʂ��z���Ă��Ȃ��`�F�b�N
				BuffAlloc(addr+1);					// �o�b�t�@�e�ʂ��������čĊ��蓖��
			}
			pBuff[addr] = ch;
			if(nMaxAddr<addr) {						// �ő发���݃A�h���X���L�^����
				nMaxAddr = addr;
			}
			pLine += 2;								// �A�X�L�[��16�i�������̂ŁA�P�o�C�g�������ƂɂQ�o�C�g�|�C���^��i�߂�
			addr ++;
		}
	}
}

// CPU���猩���A�h���X����AROM���猩���A�h���X�ɕϊ�����B
// ROM�̌��AROM1�̃r�b�g���Ȃǂ���ROM�ԍ��AROM�A�h���X�A�o�C�g�A�h���X���v�Z����
// �{���ɂ����Ă��邩������ƕs��
void CalcRomAddress( unsigned long nAddress,	// CPU���猩���A�h���X
					int nRoms,					// ROM�̌�
					int nRomWidth,				// ROM1�̃o�C�g��(16�r�b�g���Ȃ�2)
					int &nRomNumber,			// �Y������ROM�ԍ�
					unsigned long &nRomAddress,	// ROM���猩���A�h���X(�o�X���Ɋ֌W�Ȃ��A�^�̃A�h���X�BROM�̃A�h���X�o�X�̒l�ł͂Ȃ�)
					int &nByteAddress)			// ROM���̃o�C�g�A�h���X

// |        Address                        |
// |RomAddressHIGH|XXXXXXXXXX|RomAddressLOW| ROM�A�h���X��XXXX�̕�����������āAHIGH�̕����͍��ɃV�t�g������
// |              |nRomNumber|nByteAddress |
{
	int i;
	for(i=0; !(nRoms     & 1); i++) nRoms    >>=1; nRoms = i;		// nRoms = log(nRoms)/log(2)
	for(i=0; !(nRomWidth & 1); i++) nRomWidth>>=1; nRomWidth = i;	// nRomWidth = log(nRomWidth)/log(2)

	// �e�f�[�^������o�����߂̃r�b�g�}�X�N�Ȃǂ𐶐�
	unsigned long BAmask, RNmask, RAmask, ByteShift;
	BAmask = (1<<(nRomWidth))-1;					// �o�C�g�A�h���X�p�̃}�X�N
	RAmask = ~BAmask;								// ROM�A�h���X�p�̃}�X�N
	RNmask = ((1<<(nRomWidth+nRoms))-1) & RAmask;	// ROM�ԍ��p�̃}�X�N
	ByteShift = nRomWidth;
	
	nByteAddress = nAddress & BAmask;
	nRomAddress = ((nAddress>>nRoms) & RAmask) | nByteAddress;
	nRomNumber = (nAddress & RNmask) >> ByteShift;
}

// �쐬�����������C���[�W�o�b�t�@�̒��g���A�w���ROM���AROM�f�[�^���Ƃ��ĕ�����
// MOT�t�@�C���ɕ������ďo�͂���
void motout(int nRoms, int nRomWidthInByte, CRanges &ranges)
{
	int nRWIB = nRomWidthInByte;
	if(nRoms==0) return;			// ROM�̌���0��NG
	if(nRWIB==0) return;			// ROM�̃r�b�g����0��NG

	// �w���ROM�����t�@�C�����J���B�o�̓t�@�C������ROMx.mot�ɌŒ�
	FILE **fpo = new FILE*[nRoms];		// �t�@�C���n���h���p�|�C���^���m��
	char filename[32];
	for(int i=0; i<nRoms; i++) {
		sprintf(filename, "ROM%d.mot", i);
		fpo[i] = fopen(filename, "wt");
	}

	CMotorolaS *mot = new CMotorolaS[nRoms];		// ROM�̌���CMotorolaS�I�u�W�F�N�g���m��
	CRange range;
	int RN, BA;										// RN=RomNumber, BA=ByteAddress
	unsigned long RA;								// RA=RomAddress
	char tmp[256];									// S���R�[�h��������ꎞ�I�Ɋi�[���邽�߂̃o�b�t�@

	// S���R�[�h��͎��ɓo�^���Ă������g�p�͈͑S�Ă��o�͂���
	for(i=0; i<ranges.getNumberOfItems(); i++) {
		ranges.getRange(i, range);					// i�Ԗڂ͈̔͏����擾
		for(int j=0; j<nRoms; j++) {				// ROM�̌���CMotorolaS�I�u�W�F�N�g��������
			mot[j].SetDataCount(0);					// MotorolaS�I�u�W�F�N�g�̃f�[�^�T�C�Y��0�ɏ�����
		}
		// �擾�����͈͂̊Ԃ̃f�[�^��S�ďo�͂���
		for(unsigned long adr=range.top; adr<=range.bottom; adr++) {
			CalcRomAddress(adr, nRoms, nRomWidthInByte, RN, RA, BA);		// ROM�A�h���X���v�Z
			if(mot[RN].GetDataCount()==0) {									// CMotorolaS���R�[�h�̕ێ��f�[�^����0�̂Ƃ��͏��������������Ȃ̂ŁA���[�h�A�h���X�Ȃǂ�ݒ肷��K�v������
				mot[RN].InitSRecord(RA);									// ���[�h�A�h���X�A���R�[�h�^�C�v��ݒ肷��
			}
			mot[RN].SetData(RA-mot[RN].GetLoadAddress(), pBuff[adr]);		// S�I�u�W�F�N�g�̎w��̈ʒu�Ƀf�[�^��ǉ�
			if(mot[RN].GetDataCount()>=0x10) {								// S�I�u�W�F�N�g�̕ێ��f�[�^����0x10�ɂȂ�����t�@�C���ɏo�͂��A�ێ��f�[�^���N���A����
				fputs(mot[RN].GetSRecordBuff(tmp), fpo[RN]);				// S���R�[�h��������o��
				fputs("\n", fpo[RN]);
				mot[RN].SetDataCount(0);									// �ێ��f�[�^����0�ɂ���
			}
		}
		// �t�@�C���ɏo�͂��ꂸ�Ɏc���Ă��܂��Ă���S���R�[�h�����t�@�C���ɏ����o���i�t���b�V���j
		for(j=0; j<nRoms; j++) {
			if(mot[j].GetDataCount()>0) {						// �c���Ă邩�H
				fputs(mot[j].GetSRecordBuff(tmp), fpo[j]);		// S���R�[�h��������o��
				fputs("\n", fpo[j]);
				mot[j].SetDataCount(0);
			}
		}
	}
	// �S�Ẵt�@�C���n���h�����N���[�Y
	for(i=0; i<nRoms; i++) {
		fclose(fpo[i]);
	}
	delete []fpo;	// new�Ŋm�ۂ����I�u�W�F�N�g�͊J������
	delete []mot;	// new�Ŋm�ۂ����I�u�W�F�N�g�͊J������
}

// �g�����̕\��
void usage( void )
{
	puts("");
	puts("Synopsis: mot2bin [options] inputfile [outputfile]");
	puts("Options:   -r : A number of ROMs(1, 2, 4, 8, ...)");
	puts("           -w : ROM data width in byte (1, 2, 4, 8, ...)");
	puts("           -s : Output splited ROM image file(ROMx.mot)");
	puts("           -v : Verbose mode");
	puts("           -h : Help");
	puts("Note:  1. The outputfile will assume to be 'temp.bin', if you omit the outputfile.");
	puts("ie:  C:>mot2bin -s -r2 -w1 check.mot check.bin");
	puts("     ROM 2pcs, 16bit width ROM");
}

// �^����ꂽ������𐔒l�ɕϊ����āA���̒l��2^x�̒l���ǂ����`�F�b�N����
// 2^x�ȊO�̒l�������ꍇ����rtn��Ԃ��B2^x��������A�ϊ��������l��Ԃ�
// main()�̈����]���p�̃J�X�^���֐�
int CheckPow2( char *str, int rtn )
{
	int tmp1, tmp2;
	tmp1 = atoi(str);						// �����𐔒l�ɕϊ�
	for(tmp2=1; tmp2<=0x0020; tmp2<<=1) {	// ������2^x�̒l�ɂȂ��Ă��邩�`�F�b�N
		if(tmp2==tmp1) return tmp1;
	}
	return rtn;
}

void main( int argc, char *argv[])
{
	FILE *fo;
	FILE *fi;

	char sourcefile[128], outputfile[128];		// ���̓t�@�C����

	puts("*** Motorola-S to Binary converter ***");
	puts("Copyright 1999 EASTON Co.,Ltd. All rights reserved.");

	// �I�v�V�������
	int nNumberOfRoms = 1;			// ROM�̌��i�������j
	int nRomWidthInByte = 1;		// ROM�P�̃o�X���i�o�C�g�P�ʁj
	sourcefile[0] = '\0';
	strcpy(outputfile, "temp.bin");	// �o�̓t�@�C���ȗ����̃t�@�C����
	bool fExit, fSplit;
	fExit = false;
	fSplit = false;
	fVerbose = false;
	for(int i=1; i<argc; i++) {
		if(argv[i][0]=='-' || argv[i][0]=='/') {		// �����̂P�����ڂ�'-'��'/'�Ȃ�I�v�V�����w��Ƃ݂Ȃ�
			switch(argv[i][1]) {
			case 'r':
			case 'R':		// ROM�̌�
				nNumberOfRoms = CheckPow2(argv[i]+2, nNumberOfRoms);
				break;
			case 'w':
			case 'W':		// ROM�P������̃o�X���i�o�C�g�P�ʁj
				nRomWidthInByte = CheckPow2(argv[i]+2, nRomWidthInByte);
				break;
			case 'v':
			case 'V':
				fVerbose = true;
				break;
			case 's':
			case 'S':
				fSplit = true;
				break;
			case 'h':
			case 'H':
			case '?':
				fExit = true;
				break;
			default:
				printf("Illegal option '%s'\n", argv[i]);
				fExit = true;
				break;
			}
		} else {
			if(strlen(sourcefile)==0) {
				strncpy(sourcefile, argv[i], 127);
			} else {
				strncpy(outputfile, argv[i], 127);
			}
		}
	}
	if(strlen(sourcefile)==0) {
		printf("Specify source file name\n");
		fExit = true;
	}
	if(strlen(outputfile)==0) {
		printf("Specify output file name\n");
		fExit = true;
	}
	if(fExit) {
		usage();
		exit(-1);								// �I�v�V������͂Ŗ�肪�������Ƃ��i�w���v�\���̂Ƃ����j	
	}

	// ���̓t�@�C�����I�[�v��
	if((fi = fopen(sourcefile, "rt"))==NULL) {
		puts("Couldn't open source file.");
		puts(argv[1]);
		exit(-1);
	}
	
	// �o�̓t�@�C�����I�[�v��
	if((fo = fopen(outputfile, "wb"))==NULL) {
		puts("Couldn't open output file.");
		puts(argv[2]);
		fcloseall();
		exit(-1);
	}

	// �����\��
	if(fVerbose) {
		printf("*** ROM=%dpcs, ROM DATA BUS WIDTH=%dbit ***\n", nNumberOfRoms, nRomWidthInByte*8);
		printf("INPUT %s  OUTPUT %s\n", sourcefile, outputfile);
	}

	// �����������C���[�W�o�b�t�@���蓖��
	pBuff = (unsigned char*)calloc(_ALLOC_UNIT, 1);
	if(pBuff==NULL) {
		puts("Too few memory");
		fcloseall();
		exit(-1);
	}
	nBufSiz = _ALLOC_UNIT;
	nMaxAddr = 0L;

	// ���̓t�@�C���ǂݍ���
	char line[512];
	while(1) {
		fgets(line, 511, fi);			// �P�s�ǂݍ���
		if(feof(fi)) break;
		mot2bin(line);					// S���R�[�h���������C���[�W�o�b�t�@�ɏ�������
	}

	fwrite(pBuff, 1, nMaxAddr+1, fo);	// �������C���[�W�t�@�C�������o��

	oRanges.combineAll();				// �L�^�����g�p�͈͂̂����A�����ł���͈͂���������
	
	if(fSplit) motout(nNumberOfRoms, nRomWidthInByte, oRanges);		// ���g���[��S�t�H�[�}�b�g�ŏo��

	// ���̓t�@�C���̒��Ŏg�p�����̈��\��
	if(fVerbose) {
		puts("*** Used area");
		oRanges.show();					// �g�p�͈͈ꗗ��\������
	}
	if(fVerbose) printf("*** Max address = 0x%08x\n", nMaxAddr);	// �g�p�����ő�A�h���X��\��
	printf("*** File conversion succeeded.");
	if(fVerbose) puts(" Trust me!"); else puts("");

	free(pBuff);
	fcloseall();
}
