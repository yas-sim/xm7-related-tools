#include "cfile.h"

bool CFile::Open(char *pFilename, int bFlag) 
{
	char mode[10];
	mode[0] = '\0';
	if(bFlag & FMODE_READ) {
		strcat(mode, "r");
	}
	if(bFlag & FMODE_WRITE) {
		strcat(mode, "w");
	}
	if(bFlag & FMODE_TEXT) {
		strcat(mode, "t");
	}
	if(bFlag & FMODE_BIN) {
		strcat(mode, "b");
	}
	m_pFile = fopen(pFilename, mode);
	m_nOpenMode = bFlag;
	return isValid();
}

bool CFile::Seek( unsigned long nOffset )
{
	int result;
	result = -1;
	if(isValid()) {
		result = fseek(m_pFile, nOffset, SEEK_SET);
	}
	return (result==0)?true:false;
}

long CFile::Tell( void )
{
	if(isValid()) {
		return ftell(m_pFile);
	}
	return -1;
}

unsigned char CFile::ReadByte( void )
{
	unsigned char result;
	result = 0;
	if(isValid()) {
		if(!eof()) {
			result = getc(m_pFile);
		}
	}
	return result;
}

unsigned short CFile::ReadWord( void )
{
	unsigned short result;
	result = 0;
	if(isValid()) {
		for(int i=0; i<sizeof(unsigned short); i++) {
			if(!eof(),1) {
				if(m_bEndian==true) {
					result |= (getc(m_pFile) & 0x00ff)<<((sizeof(unsigned short)-1-i)*8);	// Little
				} else {
					result |= (getc(m_pFile) & 0x00ff)<<(i*8);								// Big
				}
			}
		}
	}
	return result;
}

unsigned long CFile::ReadDword( void )
{
	unsigned long result;
	result = 0L;
	if(isValid()) {
		for(int i=0; i<sizeof(unsigned long); i++) {
			if(!eof()) {
				if(m_bEndian==true) {
					result |= (getc(m_pFile) & 0x00ff)<<((sizeof(unsigned long)-1-i)*8);	// Little
				} else {
					result |= (getc(m_pFile) & 0x00ff)<<(i*8);								// Big
				}
			}
		}
	}
	return result;
}

void CFile::WriteByte( unsigned char nData )
{
	if(isValid()) {
		putc(nData, m_pFile);
	}
}

void CFile::WriteWord( unsigned short nData )
{
	if(isValid()) {
		for(int i=0; i<sizeof(unsigned short); i++) {
			if(!eof()) {
				if(m_bEndian==false) {
					putc((nData>>(i*8)) & 0x00ff, m_pFile);
				} else {
					putc((nData>>((sizeof(unsigned short)-1-i)*8)) & 0x00ff, m_pFile);
				}
			}
		}
	}
}
void CFile::WriteDword( unsigned long nData )
{
	if(isValid()) {
		for(int i=0; i<sizeof(unsigned long); i++) {
			if(!eof()) {
				if(m_bEndian==false) {
					putc((nData>>(i*8)) & 0x00ff, m_pFile);
				} else {
					putc((nData>>((sizeof(unsigned long)-1-i)*8)) & 0x00ff, m_pFile);
				}
			}
		}
	}
}
