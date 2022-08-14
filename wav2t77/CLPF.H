#ifndef _CLPF_H_
#define _CLPF_H_

#include "CFILTER.H"

#define CLPF_DEPTH	(10)
#define CLPF_AUDIO_CENTER	(0x8000)

class CLPF : public CFilter {
protected:
	long m_Buff[CLPF_DEPTH];
	void ShiftData( void );
	unsigned m_Integrate;
public:
	CLPF() : CFilter() {
		for(int i=0; i<CLPF_DEPTH; i++) {
			m_Buff[i] = CLPF_AUDIO_CENTER;
		}
		m_Integrate = 0x8000;		// サウンドのセンター値
	}
	virtual void DataInput( unsigned long data );
	virtual unsigned long DataOutput( void );
};

#endif //_CLPF_H_
