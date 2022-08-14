#include "cprecalc.h"
#include "cfile.h"
#include "cwave.h"

#include <math.h>

#define CPRECALC_AUDIO_CENTER	(0x8000)

void CPreCalc::ShiftData( void ) {
	for(int i=CPRECALC_NOD-1; i>0; i--) {
		m_Buff[i] = m_Buff[i-1];
	}
}

void CPreCalc::DataInput( unsigned long data ) {
	ShiftData();
	m_Buff[0] = data;
}

// 蓄積データの平均と、ばらつき(標準偏差)をチェックする
void CPreCalc::CheckData( unsigned long *center, unsigned long *delta ) {
	*delta = 0L;
	*center = 0L;
	long s1=0L, s2=0L;
	long data;
	int i;
	for(i=0; i<CPRECALC_NOD; i++) {
		data  = (long)m_Buff[i];
		data -= s1;
		s1   += data/(i+1);
		s2   += i*data*data/(i+1);
	}
	s2 = (long)sqrt(s2/(i-1));
	*center = (unsigned long)s1;		// 平均値
	*delta  = (unsigned long)s2;		// 標準偏差
}

bool CPreCalc::CalcParam( char *filename, unsigned long *noiselevel, 
						 unsigned long *high, unsigned long *low, bool fPhase ) {
	CWaveFile wav;
	unsigned long center, delta;
	unsigned short wavedata;
	if(wav.Open(filename, FMODE_READ | FMODE_BIN)==false) return false;
	wav.Seek(0x3a);		// wavファイルのヘッダを飛ばす
	m_NoiseLevel = CPRECALC_NOISE_LEVEL;
	m_High = CPRECALC_AUDIO_CENTER;
	m_Low  = CPRECALC_AUDIO_CENTER;
	while(!wav.eof()) {
		wavedata = wav.ReadWavDataMono();
		if(fPhase==false) wavedata = 0xffff-wavedata;		// 位相反転
		DataInput(wavedata);
		// データのバラツキを計算
		CheckData(&center, &delta);
		// ばらつきが小さいときは、安定している期間と判断
		if(delta<CPRECALC_TORELANT) {
//			if((unsigned long)abs(center-CPRECALC_AUDIO_CENTER)>m_NoiseLevel) {
			if(abs((int)(center-CPRECALC_AUDIO_CENTER))>m_NoiseLevel) {
				if(center>CPRECALC_AUDIO_CENTER) {
					m_High = (m_High+center)/2;
				} else {
					m_Low  = (m_Low +center)/2;
				}
			}
		}
	}
	wav.Close();
	*noiselevel = m_NoiseLevel;
	*high = m_High;
	*low = m_Low;
	return false;
}
