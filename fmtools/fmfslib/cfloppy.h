#ifndef _CFLOPPY_H_
#define _CFLOPPY_H_

#include "stdafx.h"
#include "SYSDEF.H"
#include <windows.h>

class FMFSLIB_API CSector {
public:
	unsigned char m_nC;
	unsigned char m_nH;
	unsigned char m_nR;
	unsigned char m_nN;
	unsigned char m_fDensity;			/* 00:MFM 40:FM */
	unsigned char m_fDDM;				/* 00:Normal 10:DeletedDataMark */
	unsigned char m_nStatus;
	unsigned short m_nSectorSize;
	unsigned char *m_pSectorData;
	CSector() { m_pSectorData=NULL; }
	virtual ~CSector() { 
//		if(m_pSectorData!=NULL) {
//			delete []m_pSectorData;
//			m_pSectorData = NULL;
//		}
	}
};

class FMFSLIB_API CFloppy {
protected:	/* Member data */
	unsigned char	m_sDiskName[17];
	unsigned char	m_fWriteProtect;		/* 00:Not protect  10:Write protected */
	unsigned char	m_fDiskType;			/* 00:2D 10:2DD 20:2HD */
	unsigned long	m_nDiskSize;
	bool			m_fAvailable;			// false:Not available  true:Available

public:		/* Member data */
public:		/* Memver function */
	CFloppy() {
		m_sDiskName[0] = '\0';
		m_fWriteProtect = 0x00;
		m_fDiskType = 0x00;
		m_nDiskSize = 0;
		m_fAvailable = false;
	}
	virtual ~CFloppy() { };

	virtual void ShowFDInfo( bool fShowTrackOffset ) = 0;
	virtual FDC_STATUS	ReadSector( int C, int H, int R, CSector *sect ) = 0;
	virtual FDC_STATUS	WriteSector( int C, int H, int R, CSector *sect ) = 0;
	virtual bool isAvailable( void ) = 0;
};

#endif
