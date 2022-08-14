#include "CRange.h"

// ２つの範囲が重なり合っているかどうかチェックする
bool CRanges::isOverlap( unsigned long top1, unsigned long bottom1,
				unsigned long top2, unsigned long bottom2 ) {
	bool result;
	unsigned long atop, abottom;

	result = true;
	
	// 境界条件のための+1,-1演算で32ビットの数がラップラウンドしてしまうのを防ぐ
	atop   =((top1   ==         0ul) ?          0ul :    top1-1);
	abottom=((bottom1==0xfffffffful) ? 0xfffffffful : bottom1+1);

	// 重なっていない場合を検出する。それ以外の場合は重なっている
	if( top2   <atop && bottom2<atop )	 result = false;
	if( abottom<top2 && abottom<bottom2) result = false;
	return result;
}

// 指定の２つの範囲データを結合する
void CRanges::combine( unsigned long top1, unsigned long bottom1,
				unsigned long top2, unsigned long bottom2, CRange *range ) {
	range->top = (top1<top2?top1:top2);
	range->bottom = (bottom1<bottom2?bottom2:bottom1);
}

// 範囲データを追加する
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

// 登録されているデータの数を返す
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

// 登録されている範囲で、結合できるものを全部結合する(1パスだけ)
void CRanges::combineAll_1( void ) {
	CRange *refptr, *ptr, *prevptr, *tmpptr;
	refptr = pTop;
	while(refptr!=NULL) {
		prevptr = refptr;
		ptr = refptr->pNext;
		while(ptr!=NULL) {
			if(isOverlap(refptr->top, refptr->bottom, ptr->top, ptr->bottom)) {
				combine(refptr->top, refptr->bottom, ptr->top, ptr->bottom, refptr);
				// オーバーラップしていたときはprevptrは動かない
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

// 登録されている範囲で、前後の範囲が結合できるもののみ結合する
// 離れた２つの範囲が結合できる場合も結合しない
// そのため、Sレコード中に現れる範囲の順番をキープできる
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

// 登録されているデータで結合できるものを結合する（マルチパス実行）
void CRanges::combineAll( void )
{
	int num;
	do {
		num = getNumberOfItems();	// 登録されている範囲データの数を調べる
		combineAll_1();
	} while (getNumberOfItems()!=num);
}

// 登録されている範囲データを表示する
void CRanges::show( void ) {
	CRange *ptr;
	ptr = pTop;
	while(ptr!=NULL) {
		printf("%08lX , %08lX\n", ptr->top, ptr->bottom);
		ptr = ptr->pNext;
	}
}

// 指定の番号の範囲データを取得する。posは0が先頭データ
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

