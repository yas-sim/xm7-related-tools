#ifndef _CRANGE_H_
#define _CRANGE_H_

#include <stdio.h>

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
	bool isOverlap( unsigned long top1, unsigned long bottom1,
					unsigned long top2, unsigned long bottom2 );
	void combine( unsigned long top1, unsigned long bottom1,
					unsigned long top2, unsigned long bottom2, CRange *range );
	void add( unsigned long top, unsigned long bottom );
	int getNumberOfItems( void );
	void combineAll_1( void );
	void combineKeepOrder( void );
	void combineAll( void );
	void show( void );
	void getRange( int pos, CRange &range );
};

#endif // _CRANGE_H_
