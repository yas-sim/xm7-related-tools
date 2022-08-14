#ifndef _MOTOROLAS_H_
#define _MOTOROLAS_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_MOTOROLAS_LENGTH		(256)
#define MAX_MOTOROLAS_LINE_LENGTH	(2+2+8+(MAX_MOTOROLAS_LENGTH)*2+2)

class CMotorolaS {
private: //-----------------------------------------------------------------
	bool			m_fAvailable;		// �������ɐ��������true
	int				m_nRecordType;		// ���R�[�h�^�C�v
	int				m_nByteCount;		// �o�C�g�J�E���g
	unsigned long	m_nLoadAddress;		// ���[�h�A�h���X
	unsigned char	*m_pData;			// �f�[�^
	unsigned char	m_nCheckSum;		// �`�F�b�N�T��

	int				m_nDataCount;		// �f�[�^��
	char			*m_pLineBuff;		// S���R�[�h���C���o�b�t�@

private: //-----------------------------------------------------------------
	inline unsigned char Hex2Dec(unsigned char ch)	{
		return (ch<='9')?(ch-'0'):((ch & ~0x20)-'A'+10);
	}

	inline unsigned char GetHex(char *pLine, int pos) {
		return (Hex2Dec(pLine[pos])<<4) | (Hex2Dec(pLine[pos+1]));
	}

	// ���R�[�h�^�C�v���烌�R�[�h����Ԃ�
	int GetAddressLength( int nRecordType )
	{
		static int alen[11] = { 2,2,3,4,2,2,2,2,2,2,2 };	// �e���R�[�h�^�C�v���Ƃ̃A�h���X��
		if(!CheckRecordType(nRecordType)) return 0;			// �s���ȃ��R�[�h�^�C�v�̏ꍇ
		return alen[nRecordType];
	}

	// ���R�[�h�^�C�v�Ƃ��Đ������ԍ����ǂ������ׂ�true/false��Ԃ�
	bool CheckRecordType( int nRecordType )
	{
		if(0<=nRecordType && nRecordType<=9) return true;
		return false;
	}

	// pLine�̍Ō�ɂP�����ǉ�����
	void AddChar( char *pLine, unsigned char ch )
	{
		if(strlen(pLine)+1<MAX_MOTOROLAS_LENGTH-1) {
			char tmp[2];
			tmp[0] = ch;
			tmp[1] = NULL;
			strcat(pLine, tmp);
		}
	}

	// pLine�̍Ō��16�i����ǉ�����
	void AddHex( char *pLine, unsigned char data )
	{
		if(strlen(pLine)+2<MAX_MOTOROLAS_LENGTH-1) {
			char tmp[3];
			sprintf(tmp, "%02X", data);
			strcat(pLine, tmp);
		}
	}

	// �����o�[�̐ݒ�l����`�F�b�N�T�����v�Z����
	// ���C���o�b�t�@�̒l(m_pLineBuff)�͎g�p���Ȃ�
	unsigned char CalcCheckSum( void );
	void GenerateSRecord( char *pLine );
	bool AnalyzeSRecord( char *pLine );

public: //-----------------------------------------------------------------
	CMotorolaS() {
		m_fAvailable = false;
		m_nRecordType = 0;
		m_nByteCount = 0;
		m_nDataCount = 2 + 0 + 1;	// �A�h���X���{�f�[�^���{�`�F�b�N�T����
		m_nLoadAddress = 0;
		m_pData = new unsigned char[MAX_MOTOROLAS_LENGTH];
		if(m_pData==NULL) return;			// �f�[�^�̈���m�ۂł��Ȃ������̂Ŏg�p�s��
		m_pLineBuff = new char[MAX_MOTOROLAS_LINE_LENGTH+1];
		if(m_pLineBuff==NULL) return;		// �f�[�^�̈���m�ۂł��Ȃ������̂Ŏg�p�s��
		m_fAvailable = true;
		m_nCheckSum = CalcCheckSum();		// �`�F�b�N�T�����v�Z������
		GenerateSRecord(m_pLineBuff);		// S���R�[�h�𐶐�����
	}

	virtual ~CMotorolaS() {
		if(m_pData!=NULL) delete []m_pData;
		if(m_pLineBuff!=NULL) delete []m_pLineBuff;
	}

	int GetRecordType( void )				{ return m_fAvailable	?m_nRecordType	:-1; }
	int GetByteCount( void )				{ return m_fAvailable	?m_nByteCount	:-1; }
	int GetDataCount( void )				{ return m_fAvailable	?m_nDataCount	:-1; }
	unsigned long GetLoadAddress( void )	{ return m_fAvailable	?m_nLoadAddress	:0xffffffff; }
	unsigned char GetCheckSum( void )		{ return m_fAvailable	?m_nCheckSum	:0xff; }
	unsigned char GetData( int nPos );
	void SetRecordType( int nType );
	void SetByteCount( int nCount );
	void SetDataCount( int nCount );
	void SetLoadAddress( unsigned long nAddress );
	void SetData( int nPos, unsigned char nData );
	void SetDataBuff( int n, unsigned char *pData );
	void SetSRecord( char *pLine );
	unsigned char *GetDataBuff( unsigned char *pBuff );
	char *GetSRecordBuff( char *pBuff );
	void InitSRecord( unsigned long nAddress );
	void AddData( unsigned char data );
};

#endif // _MOTOROLAS_H_
