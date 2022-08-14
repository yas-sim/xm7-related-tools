#ifndef _CFILE_H_
#define _CFILE_H_

#include <stdio.h>
#include <string.h>

//--------------------------------------------------------------------------------
const int FMODE_READ=	(1<<0);
const int FMODE_WRITE=	(1<<1);
const int FMODE_TEXT=	(1<<2);
const int FMODE_BIN=	(1<<3);

class CFile {
protected:
	FILE *m_pFile;
	int m_nOpenMode;
	bool m_bEndian;			// false=Little, true=Big
public:
	CFile() {
		InitMember();
	}
	virtual ~CFile() {
		Close();
	}
	bool isValid( void ) { return (m_pFile==NULL)?false:true; }
	virtual bool Open(char *pFilename, int bFlag);
	virtual void Close( void ) {
		if(isValid()) fclose(m_pFile);
	}
	virtual bool Seek(unsigned long nOffset);
	virtual long Tell( void );
	bool eof( void ) {	return (feof(m_pFile)!=0); }
	void SetBigEndian( void )		{ m_bEndian = true;	}
	void SetLittleEndian( void )	{ m_bEndian = false;	}
	unsigned char ReadByte( void );
	unsigned short ReadWord( void );
	unsigned long ReadDword( void );
	void WriteByte( unsigned char nData );
	void WriteWord( unsigned short nData );
	void WriteDword( unsigned long nData );
protected:
	void InitMember( void )
	{
		m_pFile = NULL;
		m_nOpenMode = 0;
		m_bEndian = false;		// default = little
	}
};

#endif	// _CFILE_H_
