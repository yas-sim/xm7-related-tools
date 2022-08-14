#include "CMotorolaS.h"

// �����o�[�̐ݒ�l����`�F�b�N�T�����v�Z����
// ���C���o�b�t�@�̒l(m_pLineBuff)�͎g�p���Ȃ�
unsigned char CMotorolaS::CalcCheckSum( void )
{
	if(!m_fAvailable) return 0;				// ���������s���Ă���
	m_nCheckSum = m_nByteCount;
	// �A�h���X���A���R�[�h�^�C�v�ʂɑ����Z����
	switch(m_nRecordType) {
	case 3:
	case 7:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >>24) & 0x00ffu);
		// break�Ȃ�
	case 2:
	case 8:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >>16) & 0x00ffu);
		// break�Ȃ�
	case 0:
	case 1:
	case 9:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >> 8) & 0x00ffu);
		m_nCheckSum+=(unsigned char) (m_nLoadAddress       & 0x00ffu);
		break;
	default:
		m_nCheckSum = 0;
		break;
	}
	// �f�[�^���`�F�b�N�T���ɉ��Z����
	for(int i=0; i<m_nDataCount; i++) {
		m_nCheckSum += m_pData[i];
	}
	m_nCheckSum = (unsigned char)(~m_nCheckSum);	// �o�C�g�J�E���g�{�A�h���X�{�f�[�^�̂P�̕␔
	return m_nCheckSum;
}

// �����o�ϐ��̒l���g����pLine��S���R�[�h�𐶐�����
void CMotorolaS::GenerateSRecord( char *pLine )
{
	int nCount = 0;					// S���R�[�h�̕��������J�E���g����BMAX_MOTOROLAS_LENGTH�𒴂��Ȃ��悤�ɊǗ�
	if(!m_fAvailable) return;
	pLine[0] = NULL;
	AddChar(pLine, 'S');					// S���R�[�h�̃��R�[�h�w�b�_
	AddChar(pLine, m_nRecordType + '0');	// ���R�[�h�^�C�v
	AddHex(pLine, m_nByteCount);			// �o�C�g�J�E���g
	// �A�h���X�o��
	switch(m_nRecordType) {
	case 3:
	case 7:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >> 24) & 0x00ff));
		// break�Ȃ�
	case 2:
	case 8:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >> 16) & 0x00ff));
		// break�Ȃ�
	case 0:
	case 1:
	case 9:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >>  8) & 0x00ff));
		AddHex(pLine, (unsigned char)((m_nLoadAddress      ) & 0x00ff));
		break;
	default:
		break;
	}
	// �f�[�^�o��
	for(int i=0; i<m_nDataCount; i++) {
		AddHex(pLine, m_pData[i]);
	}
	AddHex(pLine, m_nCheckSum);				// �`�F�b�N�T��
}

	// pLine��S���R�[�h�Ƃ��ĉ�͂��āA�p�����[�^�������o�ϐ��Ɋi�[����
bool CMotorolaS::AnalyzeSRecord( char *pLine )
{
	int				nRecordType;		// ���R�[�h�^�C�v
	int				nByteCount;			// �o�C�g�J�E���g
	unsigned long	nLoadAddress;		// ���[�h�A�h���X
	unsigned char	nCheckSum;			// �`�F�b�N�T��
	int				nReadPtr;			// ��͈ʒu
	int				nDataCount;
	unsigned char	ch;

	if(!m_fAvailable) return false;						// ���������s���Ă���
	if(m_pLineBuff[0]!='S') return false;				// S���R�[�h�ł͂Ȃ�
	nRecordType = Hex2Dec(pLine[1]);
	if(!CheckRecordType(nRecordType)) return false;		// ���R�[�h�^�C�v���s��
	nByteCount = GetHex(pLine, 2);
	nDataCount = nByteCount - GetAddressLength(nRecordType) -1;	// �f�[�^�����Z�o
	if(nDataCount>MAX_MOTOROLAS_LENGTH) return false;	// S���R�[�h����������
	nCheckSum = nByteCount;								// �`�F�b�N�T���ɑ���
	// ���[�h�A�h���X�擾
	nReadPtr = 4;
	nLoadAddress = 0L;
	for(int i=0; i<GetAddressLength(nRecordType); i++) {
		ch = GetHex(pLine, nReadPtr);
		nLoadAddress = (nLoadAddress<<8) + ch;
		nCheckSum += ch;								// �`�F�b�N�T���ɑ���
		nReadPtr += 2;
	}
	unsigned char *pTmpBuff = new unsigned char[nDataCount];	// �ꎞ�I�f�[�^�o�b�t�@���m�ۂ���
	if(pTmpBuff==NULL) return false;					// ������������Ȃ��c
	for(i=0; i<nDataCount; i++) {
		ch = GetHex(pLine, nReadPtr);
		pTmpBuff[i] = ch;
		nCheckSum += ch;
		nReadPtr += 2;
	}
	// �`�F�b�N�T���̔�r
	nCheckSum = (unsigned char)(~nCheckSum);
	if(nCheckSum != GetHex(pLine, nReadPtr)) {
		delete []pTmpBuff;					// ������ƈꎞ�f�[�^�o�b�t�@���J�����Ă��烊�^�[��
		return false;
	}
	// �`�F�b�N�T�����m�F�ł����̂ŁA�����o�ϐ��ɉ�͌��ʂ��R�s�[����
	m_nRecordType = nRecordType;
	SetDataCount(nDataCount);	// m_nRecordType�̒l�ƃf�[�^������m_nByteCount�������ݒ�
	m_nLoadAddress = nLoadAddress;
	for(i=0; i<nDataCount; i++) {
		m_pData[i] = pTmpBuff[i];
	}
	m_nCheckSum = nCheckSum;
	delete []pTmpBuff;
	return true;
}

unsigned char CMotorolaS::GetData( int nPos )
{
	if(!m_fAvailable) return 0;				// ���������s���Ă���
	if(nPos>=MAX_MOTOROLAS_LENGTH) return 0;
	return m_pData[nPos];
}

// S���R�[�h�f�[�^�̃��R�[�h�^�C�v��ݒ肷��
void CMotorolaS::SetRecordType( int nType )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(!CheckRecordType(nType)) return;
	m_nRecordType = nType;
	m_nByteCount = m_nDataCount + GetAddressLength(nType) + 1;		// 1�̓`�F�b�N�T���̕�
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// S���R�[�h�f�[�^�̃o�C�g�J�E���g��ݒ肷��
void CMotorolaS::SetByteCount( int nCount )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(nCount - GetAddressLength(m_nRecordType) - 1 >= MAX_MOTOROLAS_LENGTH) return;
	m_nByteCount = nCount;
	m_nDataCount = m_nByteCount - GetAddressLength(m_nRecordType) -1;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// S���R�[�h�f�[�^�̃f�[�^����ݒ肷��
void CMotorolaS::SetDataCount( int nCount )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(nCount>=MAX_MOTOROLAS_LENGTH) return;
	m_nByteCount = nCount + GetAddressLength(m_nRecordType) + 1;	// 1�̓`�F�b�N�T���̕�
	m_nDataCount = nCount;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// S���R�[�h�f�[�^�̃��[�h�A�h���X��ݒ肷��
void CMotorolaS::SetLoadAddress( unsigned long nAddress )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	m_nLoadAddress = nAddress;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// S���R�[�h�f�[�^�̔C�ӂ̈ʒu�Ƀf�[�^����������
void CMotorolaS::SetData( int nPos, unsigned char nData )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(nPos>=MAX_MOTOROLAS_LENGTH) return;
	if(nPos>=m_nDataCount) SetDataCount(nPos+1);		// ���܂ŏ������񂾍ő�f�[�^���傫���ʒu�ɏ����������Ƃ������߁A�o�C�g�J�E���g�𑝂₷
	m_pData[nPos] = nData;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// �w��̃f�[�^�o�b�t�@��S���R�[�h�̃f�[�^�o�b�t�@�ɃR�s�[����B�o�C�g�J�E���g�������ݒ肷��
// ����ȑO�Ƀf�[�^�o�b�t�@�ɏ�����Ă������e�͖���
void CMotorolaS::SetDataBuff( int n, unsigned char *pData )
{
	unsigned char *tmp;
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(n>=MAX_MOTOROLAS_LENGTH) return;
	tmp = m_pData;
	for(int i=0; i<n; i++) {
		*tmp++ = *pData++;
	}
	SetByteCount(n);
//	GenerateSRecord(m_pLineBuff);		// SetByteCount()��GenerateSRecord()���܂�ł���̂ŕs�v
}

// �w��̃A�X�L�[�������S���R�[�h������Ƃ��Ċi�[���A��͂���B��͌��ʂ̓����o�ϐ��Ɋi�[����B
void CMotorolaS::SetSRecord( char *pLine )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	strncpy(m_pLineBuff, pLine, MAX_MOTOROLAS_LINE_LENGTH);
	if(!AnalyzeSRecord(m_pLineBuff)) {
		InitSRecord(0);					// S���R�[�h����͂������ʈُ�ȃ��R�[�h�ł��邱�Ƃ����������̂ŁA�I�u�W�F�N�g������������
	}
}

// �f�[�^�o�b�t�@�̃f�[�^���R�s�[���A���̐擪�A�h���X��Ԃ�
// �^����ꂽ�R�s�[��̃A�h���X�̓f�[�^�R�s�[�ɏ\���ȃT�C�Y���m�ۂ���Ă���K�v����
unsigned char *CMotorolaS::GetDataBuff( unsigned char *pBuff )
{
	unsigned char *src, *dst;
	if(!m_fAvailable) return 0;				// ���������s���Ă���
	src = m_pData;
	dst = pBuff;
	for(int i=0; i<m_nDataCount; i++) {
		*dst++ = *src++;
	}
	return pBuff;
}

// S���R�[�h�o�b�t�@�̃f�[�^���R�s�[���A���̐擪�A�h���X��Ԃ�
// �^����ꂽ�R�s�[��̃A�h���X�̓f�[�^�R�s�[�ɏ\���ȃT�C�Y���m�ۂ���Ă���K�v����
char *CMotorolaS::GetSRecordBuff( char *pBuff )
{
	if(!m_fAvailable) return 0;				// ���������s���Ă���
	strcpy(pBuff, m_pLineBuff);
	return pBuff;
}

// �w��̃A�h���X����n�܂�悤��S���R�[�h�f�[�^������������
// �A�h���X�l���`�F�b�N���āA�K�؂ȃ��R�[�h�^�C�v��I������
void CMotorolaS::InitSRecord( unsigned long nAddress )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	m_nRecordType = 1;
	if(nAddress+MAX_MOTOROLAS_LENGTH > 0x00010000) m_nRecordType = 2;
	if(nAddress+MAX_MOTOROLAS_LENGTH > 0x01000000) m_nRecordType = 3;
	SetDataCount(0);
	m_nLoadAddress = nAddress;
	m_nCheckSum = CalcCheckSum();
}

// S���R�[�h�̃f�[�^�̍Ō�Ƀf�[�^��ǉ�����
void CMotorolaS::AddData( unsigned char data )
{
	if(!m_fAvailable) return;				// ���������s���Ă���
	if(m_nDataCount>=MAX_MOTOROLAS_LENGTH) return;
	SetData(m_nDataCount, data);
}
