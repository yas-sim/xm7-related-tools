#ifndef _CPRECALC_H_
#define _CPRECALC_H_

// FM-7セーブデータのwavファイルをスキャンして、ノイズレベル、H,Lの各レベルを
// 求めるクラス

#define CPRECALC_NOD				(10)
#define CPRECALC_TORELANT			(120)		// 標準偏差の許容範囲
#define CPRECALC_AUDIO_CENTER		(0x8000)
#define CPRECALC_NOISE_LEVEL		(0x200)

class CPreCalc {
protected:
	unsigned long m_Buff[CPRECALC_NOD];
	unsigned long m_High, m_Low, m_NoiseLevel;
public:
	CPreCalc() {
		for(int i=0; i<CPRECALC_NOD; i++) {
			m_Buff[i] = CPRECALC_AUDIO_CENTER;
		}
		m_Low = 0L;
		m_High = 0L;
		m_NoiseLevel = 0L;
	}
	virtual ~CPreCalc() {};
	bool CalcParam( char *filename, unsigned long *noiselevel, 
		unsigned long *high, unsigned long *low, bool fPhase);
	void CheckData( unsigned long *center, unsigned long *delta );
	void DataInput( unsigned long data );
	void ShiftData( void );

};

#endif // _CPRECALC_H_
