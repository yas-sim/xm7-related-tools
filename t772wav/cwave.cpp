#include "cwave.h"

void CWaveFile::WriteHeader( void )
{
	long pos;
	if(isValid()) {
		pos = CFile::Tell();
		CFile::Seek(0);
		WriteByte('R'); WriteByte('I'); WriteByte('F'); WriteByte('F');
		WriteDword(m_nNOSndData + 0x20);
		WriteByte('W'); WriteByte('A'); WriteByte('V'); WriteByte('E');
		WriteByte('f'); WriteByte('m'); WriteByte('t'); WriteByte(' ');
		WriteDword(m_nNOWavefmt);
		WriteWord(m_nDataType);		// PCM
		WriteWord(m_nNOChannel);	// Stereo
		WriteDword(m_nSplRate);
		WriteDword(m_nBytePerSec);
		WriteWord(m_nUnitSize);
		WriteWord(m_nBitPerSpl);
		WriteByte('d'); WriteByte('a'); WriteByte('t'); WriteByte('a');
		WriteDword(m_nNOSndData);	
		CFile::Seek(pos);			// 位置を元にもどす
	}
}

void CWaveFile::ReadHeader( void )
{
	long pos;
	if(isValid()) {
		pos = CFile::Tell();
		CFile::Seek(0x10);
		m_nNOWavefmt	= ReadDword();
		m_nDataType		= ReadWord();
		m_nNOChannel	= ReadWord();
		m_nSplRate		= ReadDword();
		m_nBytePerSec	= ReadDword();
		m_nUnitSize		= ReadWord();
		m_nBitPerSpl	= ReadWord();
		CFile::Seek(0x28);
		m_nNOSndData	= ReadDword();
		CFile::Seek(pos);
	}
}

bool CWaveFile::Open(char *pFilename, int bFlag)
{
	InitMember();
	if(bFlag & FMODE_READ) {
		if(CFile::Open(pFilename, bFlag)) {
			ReadHeader();
		} else {
			return false;
		}
		return true;
	} else if(bFlag & FMODE_WRITE) {
		if(CFile::Open(pFilename, bFlag)) {
			for(int i=0; i<0x2c; i++) {		// Dummyのヘッダーを書き込んでおく
				WriteByte(0);
			}
		} else {
			return false;
		}
		return true;
	}
	return false;
}

void CWaveFile::Close( void )
{
	if(m_nOpenMode & FMODE_WRITE) {
		WriteHeader();
	}
	CFile::Close();
}

// ビット/サンプルの値にしたがって１サンプリング分のデータを書き込む
void CWaveFile::WriteData( unsigned long data )
{
	switch(m_nBitPerSpl) {
	case 8:
		WriteByte((unsigned char)data);
		break;
	case 16:
		WriteWord((unsigned short)data);
		break;
	default:
		break;
	}
}

// Bit/Splの値にしたがってデータを読み出す
unsigned long CWaveFile::ReadData( void )
{
	switch(m_nBitPerSpl) {
	case 8:
		return (unsigned long)ReadByte();
	case 16:
		return (unsigned long)((unsigned short)ReadWord());
	default:
		break;
	}
	return 0L;
}

void CWaveFile::WriteSnd( unsigned long left, unsigned long right )
{
	if(m_nNOChannel==2) {
		WriteData(left);
		WriteData(right);
		m_nNOSndData+=m_nUnitSize;
	}
}

void CWaveFile::WriteSnd( unsigned long sound )
{
	if(m_nNOChannel==1) {
		WriteData(sound);
		m_nNOSndData+=m_nUnitSize;
	}
}

void CWaveFile::ReadSnd( unsigned long &left, unsigned long &right )
{
	if(m_nNOChannel==2) {
		left	= ReadData();
		right	= ReadData();
	}
}

void CWaveFile::ReadSnd( unsigned long &sound )
{
	if(m_nNOChannel==1) {
		sound	= ReadData();
	}
}

unsigned short CWaveFile::MixDown( unsigned long left, unsigned long right ) {
	return (unsigned short)((left+right)/2);
}

// つねに16ビットモノラルとして音声データを読み出す
// 音声ファイルがステレオの場合、モノラルにミックスダウンし、
// 8ビットサンプリングの場合、16ビットに拡張する
unsigned short CWaveFile::ReadWavDataMono( void ) {
	unsigned short result;
	if(m_nNOChannel==2) {
		// ステレオの時
		unsigned long left, right;
		ReadSnd(left, right);		// サウンドデータ読み込み
		if(m_nBitPerSpl==8) {
			// 8ビットサンプル時は256倍して16ビットサンプル値とあわせる
			left  <<= 8;
			right <<= 8;
		}
		result = MixDown(left, right);	// モノラルにミックスダウン
	} else {
		// モノラルのとき
		unsigned long snd;
		ReadSnd(snd);				// サウンドデータ読み込み
		if(m_nBitPerSpl==8) {
			// 8ビットサンプル時は256倍して16ビットサンプル値とあわせる
			snd <<= 8;
		}
		result = (unsigned short)snd;
	}
	return result;
}