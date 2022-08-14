#include "cagc.h"
#include <math.h>

#define SIGN(num)	(num<0?-1:num>0?1:0)

#define CAGC_AUDIO_MAX	(CAGC_AUDIO_CENTER*50/100)	// 最大値の50%を目標にゲイン設定する
#define CAGC_GAIN_LIMIT	(8*0x100)					// 最大ゲイン

#include <stdio.h>

void CAGC::DataInput( unsigned long data ) {
	int delta;
	m_latestdata = (long)data;						// 最新データを保存
	data = abs((int)(data-CAGC_AUDIO_CENTER));				// データの絶対値を取る
	delta = m_olddata - data;
	if(SIGN(delta)==-1 && SIGN(m_olddelta)==1) {	// データのピークを検出
		m_lpf.DataInput(data);
		int gain;
		gain = (CAGC_AUDIO_MAX*0x100)/m_lpf.DataOutput();		// ゲインを計算
		m_gain += (gain-m_gain)/10;								// ゲイン補正
		if(m_gain>CAGC_GAIN_LIMIT) m_gain = CAGC_GAIN_LIMIT;	// ゲインリミッタ
	}
	m_olddelta = delta;
	m_olddata = data;
}

unsigned long CAGC::DataOutput( void ) {
	short snd;
	unsigned long result;
	// デカップリング(DC成分除去)
	snd = m_latestdata-CAGC_AUDIO_CENTER;
	// 振幅補正
	snd = (snd*m_gain)/0x100;
	// クリッピング処理
	if(snd<(-CAGC_AUDIO_CENTER)+1)	snd = (-CAGC_AUDIO_CENTER)+1;
	if(snd>CAGC_AUDIO_CENTER-1)		snd = CAGC_AUDIO_CENTER-1;
	// バイアス付加
	result = snd + CAGC_AUDIO_CENTER;
//	printf("%x -> %x = %x\n", m_latestdata, m_gain, result);
	return result;
}
