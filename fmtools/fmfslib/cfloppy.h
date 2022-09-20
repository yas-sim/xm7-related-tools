#ifndef _CFLOPPY_H_
#define _CFLOPPY_H_

#include <string>
#include <vector>

#include "sysdef.h"
#include "fmfslib.h"

class FMFSLIB_API CSector {
public:
	uint8_t m_nC;
	uint8_t m_nH;
	uint8_t m_nR;
	uint8_t m_nN;
	uint8_t m_nNumberOfSectors;
	uint8_t m_fDensity;			/* 00:MFM 40:FM */
	uint8_t m_fDDM;				/* 00:Normal 10:DeletedDataMark */
	uint8_t m_nStatus;
	uint16_t m_nSectorSize;
	uint8_t *m_pSectorData;
	CSector() :
			m_nC(0), m_nH(0), m_nR(0), m_nN(0),
			m_fDensity(0), m_fDDM(0),
			m_nStatus(0),
			m_nSectorSize(0),
			m_pSectorData(nullptr)
	{ }
	virtual ~CSector() {};
};

class FMFSLIB_API CFloppy {
protected:	/* Member data */
	std::string		m_sDiskName;
	uint8_t			m_fWriteProtect;		/* 00:Not protect  10:Write protected */
	uint8_t			m_fDiskType;			/* 00:2D 10:2DD 20:2HD */
	size_t			m_nDiskSize;
	bool			m_fAvailable;			// false:Not available  true:Available

public:		/* Member data */
public:		/* Memver function */
	CFloppy() : 
			m_fWriteProtect(0x00), 
			m_fDiskType(0x00), 
			m_nDiskSize(0), 
			m_fAvailable(false) 
	{
		m_sDiskName.clear();
	}
	virtual ~CFloppy() { };

	virtual void ShowFDInfo( bool fShowTrackOffset ) = 0;
	virtual FDC_STATUS	ReadSector( const int C, const int H, const int R, CSector &sect, size_t &numRead ) = 0;
	virtual FDC_STATUS	WriteSector( const int C, const int H, const int R, CSector &sect ) = 0;
	virtual bool isAvailable( void ) = 0;
};

#endif
