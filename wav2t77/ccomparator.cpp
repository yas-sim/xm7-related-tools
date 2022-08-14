#include "ccomparator.h"

bool CComparator::Comparate( unsigned long data ) {
	bool result = true;
	if(m_LastState==true) {
		if(data<m_ThresholdL)	result = false;
		else					result = true;
	} else {
		if(data>m_ThresholdH)	result = true;
		else					result = false;
	}
	m_LastState = result;
	return result;
}
