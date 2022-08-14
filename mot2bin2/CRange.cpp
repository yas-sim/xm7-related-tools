#include "CRange.h"

// �Q�͈̔͂��d�Ȃ荇���Ă��邩�ǂ����`�F�b�N����
bool CRanges::isOverlap( unsigned long top1, unsigned long bottom1,
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
void CRanges::combine( unsigned long top1, unsigned long bottom1,
				unsigned long top2, unsigned long bottom2, CRange *range ) {
	range->top = (top1<top2?top1:top2);
	range->bottom = (bottom1<bottom2?bottom2:bottom1);
}

// �͈̓f�[�^��ǉ�����
void CRanges::add( unsigned long top, unsigned long bottom ) {
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
int CRanges::getNumberOfItems( void ) {
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
void CRanges::combineAll_1( void ) {
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
void CRanges::combineKeepOrder( void ) {
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
void CRanges::combineAll( void )
{
	int num;
	do {
		num = getNumberOfItems();	// �o�^����Ă���͈̓f�[�^�̐��𒲂ׂ�
		combineAll_1();
	} while (getNumberOfItems()!=num);
}

// �o�^����Ă���͈̓f�[�^��\������
void CRanges::show( void ) {
	CRange *ptr;
	ptr = pTop;
	while(ptr!=NULL) {
		printf("%08lX , %08lX\n", ptr->top, ptr->bottom);
		ptr = ptr->pNext;
	}
}

// �w��̔ԍ��͈̔̓f�[�^���擾����Bpos��0���擪�f�[�^
void CRanges::getRange( int pos, CRange &range )
{
	CRange *ptr;
	ptr = pTop;
	if(pos<getNumberOfItems()) {
		for(int i=0; i<pos; i++) {
			ptr = ptr->pNext;
		}
		range.top = ptr->top;
		range.bottom = ptr->bottom;
		range.pNext = NULL;
	}
}

