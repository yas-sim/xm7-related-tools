// v0.1 : MinAddr���L�^���AMinAddr-MaxAddr�͈̔͂������o�͂���悤�ɕύX
//		  SH�p�ɃA�h���X�̏�ʐ��r�b�g���}�X�N���Ă����̂�p�~
// v0.2 : �v���O�����I�����̖߂�l���C��(make�΍�)

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <process.h>

#define _ALLOC_UNIT		(0x1000)



unsigned char *pBuff;
unsigned long nBufSiz;
unsigned long nMaxAddr, nMinAddr;



class CRange {
public:
	unsigned long top;
	unsigned long bottom;
	CRange *pNext;
public:
	CRange() { pNext = NULL; }
	CRange(unsigned long top, unsigned long bottom) {
		CRange::top = top;
		CRange::bottom = bottom;
		pNext = NULL;
	}
};

class CRanges {
private:
	CRange *pTop;		
public:
	CRanges() {
		pTop = NULL;
	}

	// �Q�͈̔͂��d�Ȃ荇���Ă��邩�ǂ����`�F�b�N����
	bool isOverlap( unsigned long top1, unsigned long bottom1,
					unsigned long top2, unsigned long bottom2 ) {
		bool result;
		unsigned long atop, abottom;

		result = true;
		
		// ���E�����̂��߂�+1,-1���Z��32�r�b�g�̐������b�v���E���h���Ă��܂��̂�h��
		atop   =((top1   ==         0ul) ?          0ul :    top1-1);
		abottom=((bottom1==0xfffffffful) ? 0xfffffffful : bottom1+1);

		// �d�Ȃ��Ă��Ȃ��ꍇ�����o����B����ȊO�̏ꍇ�͏d�Ȃ��Ă���
		if( top2   <atop && bottom2<atop )	 result = false;
		if( abottom<top2 && abottom<bottom2) result = false;
		return result;
	}

	// �w��̂Q�͈̔̓f�[�^����������
	void combine( unsigned long top1, unsigned long bottom1,
					unsigned long top2, unsigned long bottom2, CRange *range ) {
		range->top = (top1<top2?top1:top2);
		range->bottom = (bottom1<bottom2?bottom2:bottom1);
	}

	// �͈̓f�[�^��ǉ�����
	void add( unsigned long top, unsigned long bottom ) {
		CRange *ptr, *prevptr;
		ptr = pTop;
		prevptr = ptr;
		if(ptr==NULL) {
			pTop = new CRange(top, bottom);
		} else {
			while(ptr!=NULL) {
				prevptr = ptr;
				ptr = ptr->pNext;
			}
			prevptr->pNext = new CRange(top, bottom);
		}
	}

	// �o�^����Ă���f�[�^�̐���Ԃ�
	int getNumberOfItems( void ) {
		int result = 0;
		CRange *ptr;
		ptr = pTop;
		while(ptr!=NULL) {
			result++;
			ptr = ptr->pNext;
		}
		return result;
	}

	// �o�^����Ă���͈͂ŁA�����ł�����̂�S����������(1�p�X����)
	void combineAll_1( void ) {
		CRange *refptr, *ptr, *prevptr, *tmpptr;
		refptr = pTop;
		while(refptr!=NULL) {
			prevptr = refptr;
			ptr = refptr->pNext;
			while(ptr!=NULL) {
				if(isOverlap(refptr->top, refptr->bottom, ptr->top, ptr->bottom)) {
					combine(refptr->top, refptr->bottom, ptr->top, ptr->bottom, refptr);
					// �I�[�o�[���b�v���Ă����Ƃ���prevptr�͓����Ȃ�
					tmpptr = ptr;
					prevptr->pNext = ptr->pNext;
					ptr = ptr->pNext;
					delete tmpptr;
				} else {
					prevptr = ptr;			
					ptr = ptr->pNext;
				}
			}
			refptr = refptr->pNext;
		}
	}

	// �o�^����Ă���͈͂ŁA�O��͈̔͂������ł�����̂̂݌�������
	// ���ꂽ�Q�͈̔͂������ł���ꍇ���������Ȃ�
	// ���̂��߁AS���R�[�h���Ɍ����͈͂̏��Ԃ��L�[�v�ł���
	void combineKeepOrder( void ) {
		CRange *refptr, *tmpptr;
		refptr = pTop;
		while(refptr->pNext != NULL) {
			if(isOverlap(refptr->top, refptr->bottom, refptr->pNext->top, refptr->pNext->bottom)) {
				combine(refptr->top, refptr->bottom, refptr->pNext->top, refptr->pNext->bottom, refptr);
				tmpptr = refptr->pNext;
				refptr->pNext = refptr->pNext->pNext;
				delete tmpptr;
				continue;
			}
			refptr = refptr->pNext;
		}
	}

	// �o�^����Ă���f�[�^�Ō����ł�����̂���������i�}���`�p�X���s�j
	void combineAll( void )
	{
		int num;
		do {
			num = getNumberOfItems();	// �o�^����Ă���͈̓f�[�^�̐��𒲂ׂ�
			combineAll_1();
		} while (getNumberOfItems()!=num);
	}

	// �o�^����Ă���͈̓f�[�^��\������
	void show( void ) {
		CRange *ptr;
		ptr = pTop;
		while(ptr!=NULL) {
			printf("%08lX , %08lX\n", ptr->top, ptr->bottom);
			ptr = ptr->pNext;
		}
	}		
};




CRanges oRanges;



bool BuffAlloc( unsigned long size )
{
	if(nBufSiz<size) {
		unsigned long nAllocSize = ((size+_ALLOC_UNIT-1)/_ALLOC_UNIT)*_ALLOC_UNIT;
		unsigned char *pTmp = (unsigned char*)realloc(pBuff, nAllocSize);	// �������m�ہI
		if(pTmp==NULL) {
			puts("Memory allocation failed.");
			puts("Too few memory");
			exit(-1);
		} else {
			pBuff = pTmp;
			nBufSiz = nAllocSize;
		}
		printf("BuffMemAlloc: %08x\n", nAllocSize);
	}
	return true;
}

// �^����ꂽ16�i�P�����𐔒l�ɕϊ�����
unsigned char Hex2Bin( char ch )
{
	if(!isxdigit((int)ch)) return 0;
	if(ch>'9') {
		ch = (ch & ~0x20) - 'A' + 10;
	} else {
		ch -= '0';
	}
	return ch;
}

// �^����ꂽ�|�C���^����Q���������o���A�����8�r�b�g16�i���̕\���Ƃ��Đ��l�ɕϊ�����
unsigned char GetByte( char *ptr )
{
	return (Hex2Bin(*ptr)<<4) + Hex2Bin(*(ptr+1));
}

// �^����ꂽ�|�C���^���s���Ƃ��AS�t�H�[�}�b�g�̃A�h���X�����o��
// �A�h���X���̓o�C�g���ň���n�Ŏw�肷��
unsigned long GetAddress( char *ptr, int n )
{
	unsigned long addr;
	addr = 0;
	ptr+=4;					// �A�h���X��4�o�C�g�ڂ���n�܂�
	for(int i=0; i<n; i++) {
		addr = (addr<<8) + GetByte(ptr);
		ptr+=2;
	}
	return addr;
}

// �^����ꂽ�|�C���^��擪�Ƃ��AS�t�H�[�}�b�g�̃o�C�g�������o��
int GetRecordLength( char *ptr )
{
	return GetByte(ptr+2);
}

// �^����ꂽ�������S���R�[�h�Ƃ݂Ȃ��A��͂���
// pBuff�ɂ��̃f�[�^����������
void mot2bin( char *pLine )
{
	int ptr = 0;
	unsigned long addr;
	int adrlen;
	int length;

	switch(pLine[1]) {
	case '0':
		return;
	case '1':
		adrlen = 2;		// �A�h���X�̒���(�o�C�g�P��)
		break;
	case '2':
		adrlen = 3;
		break;
	case '3':
		adrlen = 4;
		break;
	default:
		return;
	}
	addr = GetAddress(pLine, adrlen);
	length = GetRecordLength(pLine);
	length -= adrlen + 1;					// �A�h���X�̕��ƁA�`�F�b�N�T���̕��������Ă���
//	addr = addr & ~0xe0000000;				// SH�͏�ʃA�h���X�͏o�͂���Ȃ��̂Ń}�X�N����

	oRanges.add(addr, addr+length-1);		// �g�p�͈͂��L�^����

	// 1�s���̃f�[�^���o�b�t�@�ɏ�������
	int ch;
	pLine += adrlen*2 + 2 + 2;				// �A�h���X�ƁA���R�[�h���ƃ��R�[�h�w�b�_�̕����X�L�b�v
	if(nMinAddr>addr) {						// �ŏ��������݃A�h���X���L�^����
		nMinAddr = addr;
	}
	for(int i=0; i<length; i++) {
		ch = GetByte(pLine);				// �f�[�^����P�o�C�g�ǂݍ���
		while(addr>=nBufSiz) {					// �������ރA�h���X���o�b�t�@�e�ʂ��z���Ă��Ȃ��`�F�b�N
			BuffAlloc(addr+1);					// �o�b�t�@�e�ʂ��������čĊ��蓖��
		}
		pBuff[addr] = ch;
		if(nMaxAddr<addr) {					// �ő发���݃A�h���X���L�^����
			nMaxAddr = addr;
		}
		pLine += 2;
		addr ++;
	}
}

int main( int argc, char *argv[]) {
	
	FILE *fo;
	FILE *fi;

	puts("Motorola-S to Binary converter v0.2");
	puts("Programmed by an XM7 supporter");
	if(argc<3) {
		puts("Specify the source file and destination file.");
		puts("mot2bin *.mot *.bin");
		exit(-1);
	}

	if((fi = fopen(argv[1], "rt"))==NULL) {
		puts("Couldn't open source file.");
		puts(argv[1]);
		exit(-1);
	}

	if((fo = fopen(argv[2], "wb"))==NULL) {
		puts("Couldn't open output file.");
		puts(argv[2]);
		fcloseall();
		exit(-1);
	}

	// �����o�b�t�@���蓖��
	pBuff = (unsigned char*)calloc(_ALLOC_UNIT, 1);
	if(pBuff==NULL) {
		puts("Too few memory");
		fcloseall();
		exit(-1);
	}
	nBufSiz = _ALLOC_UNIT;
	nMaxAddr = 0L;
	nMinAddr = 0xffffffffUL;

	char line[512];
	while(1) {
		fgets(line, 511, fi);			// �P�s�ǂݍ���
		if(feof(fi)) break;
		mot2bin(line);
	}
	oRanges.combineAll();				// �L�^�����g�p�͈͂̂����A�����ł���͈͂���������
	puts("*** Used area");
	oRanges.show();						// �g�p�͈͈ꗗ��\������
//	fwrite(pBuff+nMinAddr, 1, nMaxAddr-nMinAddr+1, fo);	// �����o��
	fwrite(pBuff         , 1, nMaxAddr         +1, fo);	// �����o��
	printf("*** Min addres = 0x%08x  Max address = 0x%08x\n", nMinAddr, nMaxAddr);
	puts("*** File conversion succeeded.");
	free(pBuff);
	fcloseall();
	return 0;
}
