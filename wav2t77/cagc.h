#ifndef _CAGC_H_
#define _CAGC_H_

#include "cfilter.h"
#include "clpf.h"

#define CAGC_AUDIO_CENTER	(0x8000)

class CAGC : public CFilter {
protected:
	CLPF m_lpf;
	int m_olddelta;
	unsigned long m_olddata;
	long m_latestdata;
	int m_gain;
public:
	CAGC() : CFilter() {
		m_olddelta = 0;
		m_olddata = 0;
		m_latestdata = CAGC_AUDIO_CENTER;
		m_gain = 1*0x100;
	}
	virtual void DataInput( unsigned long data );
	virtual unsigned long DataOutput( void );
};

#endif //_CAGC_H_
