#ifndef _CWAVE_H_
#define _CWAVE_H_

#include "cfile.h"

//--------------------------------------------------------------------------------
class CWaveFile : public CFile {
protected:
										// +00 4�o�C�g 'R' 'I' 'F' 'F' 
//	unsigned long	m_nTotalSize;		// +04 4�o�C�g ����ȉ��̃o�C�g�� (= �t�@�C���T�C�Y - 8) 
										// +08 8�o�C�g 'W' 'A' 'V' 'E' 'f' 'm' 't' ' '(���X�y�[�X) 
	unsigned long	m_nNOWavefmt;		// +10 4�o�C�g WAVEfmt���̃o�C�g���C�˂� 16 (10 00 00 00) 
	unsigned short	m_nDataType;		// +14 2�o�C�g �f�[�^�`�� (PCM: 01 00)
	unsigned short	m_nNOChannel;		// +16 2�o�C�g �`���l���� (���m: 01 00 �X�e���I: 02 00) 
	unsigned long	m_nSplRate;			// +18 4�o�C�g �T���v�����O���[�g (44100Hz �Ȃ� 44 AC 00 00) 
	unsigned long	m_nBytePerSec;		// +1C 4�o�C�g �o�C�g�^�b (44100Hz �X�e���I 16�r�b�g �Ȃ� 10 B1 02 00) 
	unsigned short	m_nUnitSize;		// +20 2�o�C�g �o�C�g�^�T���v���~�`���l���� (�X�e���I 16�r�b�g �Ȃ� 04 00) 
	unsigned short	m_nBitPerSpl;		// +22 2�o�C�g �r�b�g�^�T���v�� (16�r�b�g �Ȃ� 10 00) 
										// +24 4�o�C�g ���̖��O ('d' 'a' 't' 'a'�C'f' 'a' 'c' 't' �Ȃ�) 
	unsigned long m_nNOSndData;			// +28 4�o�C�g ���̗��̃o�C�g�� n 
	// +2C�` Wave�f�[�^ (L,R,L,R�̏��B8Bit:0�`255(����=128), 16bit=-32768�`+32767(����=0))

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
	unsigned long	GetNOWavefmt( void )	{ return m_nNOWavefmt; }	// 4�o�C�g WAVEfmt���̃o�C�g���C�˂� 16 (10 00 00 00) 
	unsigned short	GetDataType( void )		{ return m_nDataType; }		// 2�o�C�g �f�[�^�`�� (PCM: 01 00)
	unsigned short	GetNOChanell( void )	{ return m_nNOChannel; }	// 2�o�C�g �`���l���� (���m: 01 00 �X�e���I: 02 00) 
	unsigned long	GetSplRate( void )		{ return m_nSplRate; }		// 4�o�C�g �T���v�����O���[�g (44100Hz �Ȃ� 44 AC 00 00) 
	unsigned long	GetBytePerSec( void )	{ return m_nBytePerSec; }	// 4�o�C�g �o�C�g�^�b (44100Hz �X�e���I 16�r�b�g �Ȃ� 10 B1 02 00) 
	unsigned short	GetUnitSize( void )		{ return m_nUnitSize;}		// 2�o�C�g �o�C�g�^�T���v���~�`���l���� (�X�e���I 16�r�b�g �Ȃ� 04 00) 
	unsigned short	GetBitPerSpl( void )	{ return m_nBitPerSpl; }	// 2�o�C�g �r�b�g�^�T���v�� (16�r�b�g �Ȃ� 10 00) 
	unsigned long	GetNOSndData( void )	{ return m_nNOSndData; }	// 4�o�C�g ���̗��̃o�C�g�� n 

	unsigned short MixDown( unsigned long left, unsigned long right );
	unsigned short ReadWavDataMono( void );				// ��Ƀ��m����16�r�b�g�ɕϊ����ĉ����f�[�^��ǂݏo��(�X�e���I�̏ꍇ�����m�����Ƀ~�b�N�X�_�E��)

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
	void WriteData( unsigned long data );		// Bit/Spl�̒l�ɂ��������ăf�[�^����������
	unsigned long ReadData( void );
};

#endif
