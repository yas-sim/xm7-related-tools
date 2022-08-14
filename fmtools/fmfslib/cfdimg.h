#ifndef _CFDIMG_H_
#define _CFDIMG_H_

#include "SYSDEF.H"
#include "CFLOPPY.H"
#include <windows.h>


#define _CFDIMAGE_MAX_FILE_NAME_LENGTH_	(256)
#define _CFDIMAGE_MAX_TRACK_			(164)

class FMFSLIB_API CSectorImg : public CSector {
public:
	unsigned char m_nNumberOfSectors;
};

class FMFSLIB_API CFDImg : public CFloppy {
protected:	/* Member data */
	TCHAR	*m_pFDImageFilename;
	HANDLE	m_hFDImage;

	unsigned long m_nTrackOffset[_CFDIMAGE_MAX_TRACK_+1];

public:		/* Member data */
protected:	/* Member function */
	bool ReadHeader( void );
	long GetTrackOffset( int C, int H );
	long GetSectorOffset( int C, int H, int R );
	bool ReadSectorID( long Offset, CSector *sect );
public:		/* Memver function */
	CFDImg() : CFloppy(){
		m_pFDImageFilename = NULL;
		m_hFDImage = INVALID_HANDLE_VALUE;
	}
	virtual ~CFDImg() {
		if(m_hFDImage!=INVALID_HANDLE_VALUE) {
			CloseHandle(m_hFDImage);
		}
	}

	bool OpenFDImage( unsigned char *pFileName );
	bool CloseFDImage( void );
	virtual void ShowFDInfo( bool fShowTrackOffset );
	virtual FDC_STATUS	ReadSector( int C, int H, int R, CSector *sect );
	virtual FDC_STATUS	WriteSector( int C, int H, int R, CSector *sect );
	virtual bool isAvailable( void ) { return m_fAvailable; }
};

#endif
