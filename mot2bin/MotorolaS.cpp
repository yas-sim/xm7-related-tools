#include <stdio.h>
#include <stdlib.h>

#include "motorolas.h"



static SFORM motDATA;





#define HEX2DEC(CH)				(((CH)<='9')?((CH)-'0'):((CH)-'A'+10))
#define GETHEX(BUFFER, POS)		((HEX2DEC(BUFFER[(POS)])<<4) | (HEX2DEC(BUFFER[(POS)+1])))

const int MAXSRECORDSIZE=256;

class CMotorolaS {
private:
	int				m_nSum;					// ���݂̃`�F�b�N�T��
	unsigned long	m_nAdrs;				// ���݂̃g�b�v�A�h���X�ƃ{�g���A�h���X
	int				m_nRecordType;			// ���R�[�h�^�C�v
	int				m_nLengh;				// �f�[�^���̃o�C�g��
	bool			m_fValid;				// ���݁A�L���ȃ��R�[�h���ǂݍ��܂�Ă��邩�ǂ���
	int				m_nLineCount;			// �ǂݍ��񂾍s��
	int				m_nAdrLen;
	char			m_cpBuff[MAXSRECORDSIZE];
	char			m_cpData[MAXSRECORDSIZE/2];
	FILE			*m_fp;
	
	void _getAdrLen( void )
	{
		int alen[11] = { 2,2,3,4,2,2,2,2,2,2,2 };	// �e���R�[�h�^�C�v���Ƃ̃A�h���X��
		m_nAdrLen = alen[_getRecordType()];	
	}
	
	void _getData( char *databuff )
	{
		int i, byte, ofst;
		byte = _getByteCount()-1;
		ofst = m_nAdrLen;
		for(i=ofst; i<byte; i++) {
			*databuff++ = GETHEX(m_cpBuff, i*2+4);
		}
	}

	unsigned long _getAddress( void )
	{
		unsigned long result;
		result = 0ul;

		for(int i=0; i<m_nAdrLen; i++) {
			result = (result << 8) | GETHEX(m_cpBuff, i*2+4);
		}
		return result;
	}

	// ���R�[�h����`�F�b�N�T���𔲂��o��
	int _getSum( void )			{ return GETHEX(m_cpBuff, _getByteCount()*2+2);	}
	int _getRecordType( void )	{ return m_cpBuff[1] & 0x0f;	}
	int _getByteCount( void )	{ return GETHEX(m_cpBuff, 2);	}

public:

	CMotorolaS()
	{
		m_fp			= (FILE*)NULL;
		m_cpBuff[0]		= '\0';
		m_nSum			= -1;
		m_nAdrs			= 0UL;
		m_nRecordType	= -1;
		m_fValid		= false;
		m_nLineCount	= 0;
	}

	CMotorolaS( char *filename , char *mode )
	{
		open(filename, mode);
		CMotorolaS();
	}

	virtual ~CMotorolaS( void )
	{
		close();
	}

	// �t�@�C�����J��
	void open( char *filename , char *mode )
	{
		m_fp = fopen(filename, mode);
	}
	
	// �t�@�C�������
	void close( void )
	{
		if(m_fp!=(FILE*)NULL) fclose(m_fp);
		m_fp = (FILE*)NULL;
	}

	// �s�i�L�����N�^�j�f�[�^����`�F�b�N�T�����v�Z����
	int calcSum( void )
	{
		int sum;
		sum = 0;
		if(isValid()) {
			for(int i=0; i<_getByteCount(); i++) {
				sum+= GETHEX(m_cpBuff, i*2+2);
			}
			return (~sum) & 0x00ff;
		}
		return -2;		// getSum()���G���[����-1��Ԃ��̂ŁAcalsSum()���G���[����-1��Ԃ��ƃ`�F�b�N�T�����������Ɗ��Ⴂ�����̂�h���ړI��-2��Ԃ�
	}

	// ���݂̍s�o�b�t�@(m_cpBuff)����͂��āA�N���X�����o�e��p�����[�^��ݒ肷��
	void analyzeRecord( void )
	{
		m_nRecordType	= _getRecordType();					// ���R�[�h�^�C�v
		_getAdrLen();										// �A�h���X���̒�����adrslen�Ɋi�[����
		m_nLengh		= _getByteCount()-m_nAdrLen-1;		// �f�[�^���݂̂̃f�[�^��
		m_nAdrs			= _getAddress();					// ���R�[�h�̃A�h���X
		_getData(m_cpData);									// S���R�[�h����f�[�^�𔲂��o��
		m_nSum			= _getSum();						// S���R�[�h�ɏ�����Ă���`�F�b�N�T���l
	}

	// �t�@�C������P���R�[�h�ǂݍ��݁A�N���X�����o�Ɋe�p�����[�^��ݒ肷��
	bool getRecord( void )
	{
		m_fValid = false;
		if(m_fp!=NULL) {
			do {
				fgets(m_cpBuff, MAXSRECORDSIZE-2, m_fp);
				m_nLineCount ++;									// �ǂݍ��񂾍s�����P���₷
			} while(m_cpBuff[0]!='S' && m_cpBuff[1]!='s' && !feof(m_fp));	// S���R�[�h�ȊO�̍s��ǂݔ�΂�
			if(feof(m_fp)) {
				m_nLineCount = 0;
				m_fValid = false;
				return false;
			}
			m_fValid = true;
			analyzeRecord();
			return true;
		} else {
			m_fValid = false;
			return false;
		}
	}

	// �o�C�g�J�E���g�A���[�h�A�h���X�A�f�[�^����`�F�b�N�T�������߂�
	unsigned char calcTotalSum( void )
	{
		
	}

	void putchToBuff( char ch )
	{
	}

	void generateSRecord( void )
	{
		unsigned char *ptr;

		ptr = (unsigned char*)m_cpBuff;


	}

	// S���R�[�h�������Ɏg�p����f�[�^���R�[�h�^�C�v
	void setRecordType( int type ) {
		m_nRecordType = type;
	}

	int isValid( void )				{ return m_fValid; }
	int getLength( void )			{ return isValid() ? m_nLengh			: -1;			}
	int getLineCount( void )		{ return isValid() ? m_nLineCount		: -1;			}
	int getRecordType( void )		{ return isValid() ? m_nRecordType		: -1;			}
	int getSum( void )				{ return isValid() ? m_nSum				: -1;			}
	unsigned long getAdrs( void )	{ return isValid() ? m_nAdrs			: 0xfffffffful; }
	int getCalcSum( void )			{ return isValid() ? calcSum()			: -1;			}
	bool isFileOpen( void )			{ return (m_fp!=(FILE*)NULL) ? true		: false;		}
	char *getBuff( void )			{ return isValid() ? m_cpBuff           : "Invalid Line\n\0"; }
};




//-----------------------------------------------------------------------------------

/* SFORM�̒��g����ɂ��� */
void motInit( void )
{
	motDATA.Type=0;
	motDATA.Size=0;
	motDATA.Address=0L;
	motDATA.Sum=0;
}

/* ���R�[�h�^�C�v���Z�b�g���� */
void motSetType( unsigned char type )
{
	motDATA.Type=type;
}

/* ���R�[�h�J�n�A�h���X���Z�b�g���� */
void motSetAddress( unsigned long address  )
{
	motDATA.Address=address;
}

/* ���R�[�h���o�͂��� */
static void motPut( void )
{
	int i;
	unsigned long adr;
	unsigned char siz;

	fprintf(stdout, "S%d", motDATA.Type);
	switch(motDATA.Type) {
		case 0:
		case 1:
		case 9:	siz=3+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%04LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr	   & 0x00ffu);
				break;
		case 2:
		case 8:	siz=4+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%06LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
		case 3:
		case 7:	siz=5+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%08LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>24) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
	}
	for(i=0; i<motDATA.Size; i++ ) {
		fprintf(stdout, "%02X", motDATA.Data[i]);
	}
	fprintf(stdout, "%02X\n", (unsigned char)(~(motDATA.Sum)));		/* �f�[�^���v�̕␔���`�F�b�N�T�� */
}

/* ���܂��Ă���f�[�^���o�͂��A�o�b�t�@���t���b�V������ */
void motFlush( void )
{
	motPut();
	motDATA.Address+= motDATA.Size;
	motDATA.Size=0;
	motDATA.Sum=0;
}

/* ���R�[�h�Ƀf�[�^���X�g�b�N���� */
/* �f�[�^�����R�[�h���ɂȂ�����A����܂ł̃f�[�^���o�͂��� */
void motStock( unsigned char stkdata )
{
	motDATA.Data[motDATA.Size++]=stkdata;
	motDATA.Sum+=stkdata;
	if(motDATA.Size>=motUNIT) {
		motFlush();
	}
}