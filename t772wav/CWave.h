#ifndef _CWAVE_H_
#define _CWAVE_H_

#include "cfile.h"

//--------------------------------------------------------------------------------
class CWaveFile : public CFile {
protected:
										// +00 4バイト 'R' 'I' 'F' 'F' 
//	unsigned long	m_nTotalSize;		// +04 4バイト これ以下のバイト数 (= ファイルサイズ - 8) 
										// +08 8バイト 'W' 'A' 'V' 'E' 'f' 'm' 't' ' '(←スペース) 
	unsigned long	m_nNOWavefmt;		// +10 4バイト WAVEfmt欄のバイト数，つねに 16 (10 00 00 00) 
	unsigned short	m_nDataType;		// +14 2バイト データ形式 (PCM: 01 00)
	unsigned short	m_nNOChannel;		// +16 2バイト チャネル数 (モノ: 01 00 ステレオ: 02 00) 
	unsigned long	m_nSplRate;			// +18 4バイト サンプリングレート (44100Hz なら 44 AC 00 00) 
	unsigned long	m_nBytePerSec;		// +1C 4バイト バイト／秒 (44100Hz ステレオ 16ビット なら 10 B1 02 00) 
	unsigned short	m_nUnitSize;		// +20 2バイト バイト／サンプル×チャネル数 (ステレオ 16ビット なら 04 00) 
	unsigned short	m_nBitPerSpl;		// +22 2バイト ビット／サンプル (16ビット なら 10 00) 
										// +24 4バイト 欄の名前 ('d' 'a' 't' 'a'，'f' 'a' 'c' 't' など) 
	unsigned long m_nNOSndData;			// +28 4バイト この欄のバイト数 n 
	// +2C～ Waveデータ (L,R,L,Rの順。8Bit:0～255(無音=128), 16bit=-32768～+32767(無音=0))

	char *m_FileName;

public:
	CWaveFile() {
		InitMember();
		CFile::InitMember();
	}
	virtual ~CWaveFile() { };

	virtual bool Open(char *pFilename, int bFlag);
	virtual void Close( void );

	void WriteSnd( unsigned long left, unsigned long right );
	void WriteSnd( unsigned long sound );
	void ReadSnd( unsigned long &left, unsigned long &right );
	void ReadSnd( unsigned long &sound );
	double GetTotalTime( void ) {
		return ((double)m_nNOSndData/(double)m_nUnitSize/(double)m_nSplRate);
	}
	unsigned long	GetNOWavefmt( void )	{ return m_nNOWavefmt; }	// 4バイト WAVEfmt欄のバイト数，つねに 16 (10 00 00 00) 
	unsigned short	GetDataType( void )		{ return m_nDataType; }		// 2バイト データ形式 (PCM: 01 00)
	unsigned short	GetNOChanell( void )	{ return m_nNOChannel; }	// 2バイト チャネル数 (モノ: 01 00 ステレオ: 02 00) 
	unsigned long	GetSplRate( void )		{ return m_nSplRate; }		// 4バイト サンプリングレート (44100Hz なら 44 AC 00 00) 
	unsigned long	GetBytePerSec( void )	{ return m_nBytePerSec; }	// 4バイト バイト／秒 (44100Hz ステレオ 16ビット なら 10 B1 02 00) 
	unsigned short	GetUnitSize( void )		{ return m_nUnitSize;}		// 2バイト バイト／サンプル×チャネル数 (ステレオ 16ビット なら 04 00) 
	unsigned short	GetBitPerSpl( void )	{ return m_nBitPerSpl; }	// 2バイト ビット／サンプル (16ビット なら 10 00) 
	unsigned long	GetNOSndData( void )	{ return m_nNOSndData; }	// 4バイト この欄のバイト数 n 

	unsigned short MixDown( unsigned long left, unsigned long right );
	unsigned short ReadWavDataMono( void );				// 常にモノラル16ビットに変換して音声データを読み出す(ステレオの場合もモノラルにミックスダウン)

protected:
	void InitMember( void ) {
		m_FileName		= NULL;
		m_nNOWavefmt	= 16;
		m_nDataType		= 1;		// PCM
		m_nBitPerSpl	= 8;
		m_nNOChannel	= 1;
		m_nSplRate		= 44100;	// 44.1khz
		m_nBytePerSec	= m_nSplRate * m_nNOChannel * (m_nBitPerSpl/8);
		m_nUnitSize		= m_nNOChannel * (m_nBitPerSpl>>3);
		m_nNOSndData	= 0L;
	}
	void WriteHeader( void );
	void ReadHeader( void );
	void WriteData( unsigned long data );		// Bit/Splの値にしたがってデータを書き込む
	unsigned long ReadData( void );
};

#endif
