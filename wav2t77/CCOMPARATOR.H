#ifndef _CCOMPARATOR_H_
#define _CCOMPARATOR_H_

class CComparator {
protected:
	unsigned long m_ThresholdH;
	unsigned long m_ThresholdL;
	bool m_LastState;
public:
	CComparator() {
		m_LastState = false;
	}
	virtual ~CComparator() {};
	bool Comparate( unsigned long data );
	void SetThreshold( unsigned long th, unsigned long tl ) {
		m_ThresholdH = th;
		m_ThresholdL = tl;
	}
};

#endif // _CCOMPARATOR_H_
