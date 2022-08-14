#include "CWave.h"

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
		CFile::Seek(pos);			// �ʒu�����ɂ��ǂ�
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
			for(int i=0; i<0x2c; i++) {		// Dummy�̃w�b�_�[����������ł���
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

// �r�b�g/�T���v���̒l�ɂ��������ĂP�T���v�����O���̃f�[�^����������
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

// Bit/Spl�̒l�ɂ��������ăf�[�^��ǂݏo��
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

// �˂�16�r�b�g���m�����Ƃ��ĉ����f�[�^��ǂݏo��
// �����t�@�C�����X�e���I�̏ꍇ�A���m�����Ƀ~�b�N�X�_�E�����A
// 8�r�b�g�T���v�����O�̏ꍇ�A16�r�b�g�Ɋg������
unsigned short CWaveFile::ReadWavDataMono( void ) {
	unsigned short result;
	if(m_nNOChannel==2) {
		// �X�e���I�̎�
		unsigned long left, right;
		ReadSnd(left, right);		// �T�E���h�f�[�^�ǂݍ���
		if(m_nBitPerSpl==8) {
			// 8�r�b�g�T���v������256�{����16�r�b�g�T���v���l�Ƃ��킹��
			left  <<= 8;
			right <<= 8;
		}
		result = MixDown(left, right);	// ���m�����Ƀ~�b�N�X�_�E��
	} else {
		// ���m�����̂Ƃ�
		unsigned long snd;
		ReadSnd(snd);				// �T�E���h�f�[�^�ǂݍ���
		if(m_nBitPerSpl==8) {
			// 8�r�b�g�T���v������256�{����16�r�b�g�T���v���l�Ƃ��킹��
			snd <<= 8;
		}
		result = (unsigned short)snd;
	}
	return result;
}