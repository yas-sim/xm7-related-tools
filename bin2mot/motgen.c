#include <stdio.h>
#include <stdlib.h>

#include "motgen.h"

static SFORM motDATA;

/* SFORM�̒��g����ɂ��� */
void motInit( void )
{
	motDATA.Type=0;
	motDATA.Size=0;
	motDATA.Address=0L;
	motDATA.Sum=0;
}

/* ���R�[�h�^�C�v���Z�b�g���� */
void motSetType( unsigned char type )
{
	motDATA.Type=type;
}

/* ���R�[�h�J�n�A�h���X���Z�b�g���� */
void motSetAddress( unsigned long address  )
{
	motDATA.Address=address;
}

/* ���R�[�h���o�͂��� */
static void motPut( void )
{
	int i;
	unsigned long adr;
	unsigned char siz;

	fprintf(stdout, "S%d", motDATA.Type);
	switch(motDATA.Type) {
		case 0:
		case 1:
		case 9:	siz=3+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%04LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr	   & 0x00ffu);
				break;
		case 2:
		case 8:	siz=4+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%06LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
		case 3:
		case 7:	siz=5+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%08LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>24) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
	}
	for(i=0; i<motDATA.Size; i++ ) {
		fprintf(stdout, "%02X", motDATA.Data[i]);
	}
	fprintf(stdout, "%02X\n", (unsigned char)(~(motDATA.Sum)));		/* �f�[�^���v�̕␔���`�F�b�N�T�� */
}

/* ���܂��Ă���f�[�^���o�͂��A�o�b�t�@���t���b�V������ */
void motFlush( void )
{
	motPut();
	motDATA.Address+= motDATA.Size;
	motDATA.Size=0;
	motDATA.Sum=0;
}

/* ���R�[�h�Ƀf�[�^���X�g�b�N���� */
/* �f�[�^�����R�[�h���ɂȂ�����A����܂ł̃f�[�^���o�͂��� */
void motStock( unsigned char stkdata )
{
	motDATA.Data[motDATA.Size++]=stkdata;
	motDATA.Sum+=stkdata;
	if(motDATA.Size>=motUNIT) {
		motFlush();
	}
}
