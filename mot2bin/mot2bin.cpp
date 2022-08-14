// v0.1 : MinAddrを記録し、MinAddr-MaxAddrの範囲だけを出力するように変更
//		  SH用にアドレスの上位数ビットをマスクしていたのを廃止
// v0.2 : プログラム終了時の戻り値を修正(make対策)

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
//#include <process.h>

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

	// ２つの範囲が重なり合っているかどうかチェックする
	bool isOverlap( unsigned long top1, unsigned long bottom1,
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
	void combine( unsigned long top1, unsigned long bottom1,
					unsigned long top2, unsigned long bottom2, CRange *range ) {
		range->top = (top1<top2?top1:top2);
		range->bottom = (bottom1<bottom2?bottom2:bottom1);
	}

	// 範囲データを追加する
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

	// 登録されているデータの数を返す
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

	// 登録されている範囲で、結合できるものを全部結合する(1パスだけ)
	void combineAll_1( void ) {
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

	// 登録されているデータで結合できるものを結合する（マルチパス実行）
	void combineAll( void )
	{
		int num;
		do {
			num = getNumberOfItems();	// 登録されている範囲データの数を調べる
			combineAll_1();
		} while (getNumberOfItems()!=num);
	}

	// 登録されている範囲データを表示する
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
		unsigned char *pTmp = (unsigned char*)realloc(pBuff, nAllocSize);	// メモリ確保！
		if(pTmp==NULL) {
			puts("Memory allocation failed.");
			puts("Too few memory");
			exit(-1);
		} else {
			pBuff = pTmp;
			nBufSiz = nAllocSize;
		}
		printf("BuffMemAlloc: %08lx\n", nAllocSize);
	}
	return true;
}

// 与えられた16進１文字を数値に変換する
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

// 与えられたポインタから２文字を取り出し、それを8ビット16進数の表現として数値に変換する
unsigned char GetByte( char *ptr )
{
	return (Hex2Bin(*ptr)<<4) + Hex2Bin(*(ptr+1));
}

// 与えられたポインタを行頭とし、Sフォーマットのアドレスを取り出す
// アドレス長はバイト数で引数nで指定する
unsigned long GetAddress( char *ptr, int n )
{
	unsigned long addr;
	addr = 0;
	ptr+=4;					// アドレスは4バイト目から始まる
	for(int i=0; i<n; i++) {
		addr = (addr<<8) + GetByte(ptr);
		ptr+=2;
	}
	return addr;
}

// 与えられたポインタを先頭とし、Sフォーマットのバイト数を取り出す
int GetRecordLength( char *ptr )
{
	return GetByte(ptr+2);
}

// 与えられた文字列をSレコードとみなし、解析する
// pBuffにそのデータを書き込む
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
		adrlen = 2;		// アドレスの長さ(バイト単位)
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
	length -= adrlen + 1;					// アドレスの分と、チェックサムの分を引いておく
//	addr = addr & ~0xe0000000;				// SHは上位アドレスは出力されないのでマスクする

	oRanges.add(addr, addr+length-1);		// 使用範囲を記録する

	// 1行分のデータをバッファに書き込む
	int ch;
	pLine += adrlen*2 + 2 + 2;				// アドレスと、レコード長とレコードヘッダの分をスキップ
	if(nMinAddr>addr) {						// 最小書き込みアドレスを記録する
		nMinAddr = addr;
	}
	for(int i=0; i<length; i++) {
		ch = GetByte(pLine);				// データから１バイト読み込む
		while(addr>=nBufSiz) {					// 書き込むアドレスがバッファ容量を越えていなかチェック
			BuffAlloc(addr+1);					// バッファ容量を割増して再割り当て
		}
		pBuff[addr] = ch;
		if(nMaxAddr<addr) {					// 最大書込みアドレスを記録する
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

	// 初期バッファ割り当て
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
		fgets(line, 511, fi);			// １行読み込み
		if(feof(fi)) break;
		mot2bin(line);
	}
	oRanges.combineAll();				// 記録した使用範囲のうち、結合できる範囲を結合する
	puts("*** Used area");
	oRanges.show();						// 使用範囲一覧を表示する
//	fwrite(pBuff+nMinAddr, 1, nMaxAddr-nMinAddr+1, fo);	// 書き出し
	fwrite(pBuff         , 1, nMaxAddr         +1, fo);	// 書き出し
	printf("*** Min addres = 0x%08lx  Max address = 0x%08lx\n", nMinAddr, nMaxAddr);
	puts("*** File conversion succeeded.");
	free(pBuff);
	fcloseall();
	return 0;
}
