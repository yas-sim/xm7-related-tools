#ifndef _CFILTER_H_
#define _CFILTER_H_


class CFilter {
public:
	CFilter() {};
	virtual ~CFilter() {};
	virtual void DataInput( unsigned long data ) = 0;
	virtual unsigned long DataOutput( void ) = 0;
};

#endif //_CFILTER_H_
