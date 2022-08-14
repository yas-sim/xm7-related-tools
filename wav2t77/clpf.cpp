#include "clpf.h"

#define LPFTYPE	(1)

// データを後ろにずらす
void CLPF::ShiftData( void ) {
	for(int i=CLPF_DEPTH-1; i>0; i--) {
		m_Buff[i] = m_Buff[i-1];
	}
}

void CLPF::DataInput( unsigned long data ) {
	ShiftData();
#if LPFTYPE==1
	m_Buff[0] = data;
#elif LPFTYPE==2
	m_Integrate = m_Integrate + ((data-m_Integrate)>>3);
#endif
}

unsigned long CLPF::DataOutput( void ) {
	unsigned long result = 0L;
#if LPFTYPE==1
	for(int i=0; i<CLPF_DEPTH; i++) {
		result += (m_Buff[i]/(2<<i));
	}
	return result;
#elif LPFTYPE==2
	return m_Integrate;
#endif
}
