#ifndef _CFDIMG_H_
#define _CFDIMG_H_

#include <vector>
#include <string>
#include <fstream>

#include "sysdef.h"
#include "cfloppy.h"

#define _CFDIMAGE_MAX_FILE_NAME_LENGTH_	(256)
#define _CFDIMAGE_MAX_TRACK_			(164)

class FMFSLIB_API CSectorImg : public CSector {
public:
	unsigned char m_nNumberOfSectors;
};

class FMFSLIB_API CFDImg : public CFloppy {
protected:	/* Member data */
	std::string			m_pFDImageFilename;
	std::fstream		m_hFDImage;

	std::vector<size_t>	m_nTrackOffset;

public:		/* Member data */
protected:	/* Member function */
	bool ReadHeader( void );
	long GetTrackOffset( int C, int H );
	long GetSectorOffset( int C, int H, int R );
	bool ReadSectorID( long Offset, CSector &sect );
public:		/* Memver function */
	CFDImg() : CFloppy(){
		m_nTrackOffset.resize(_CFDIMAGE_MAX_TRACK_);
		m_pFDImageFilename.clear();
	}
	virtual ~CFDImg() {
		if(m_hFDImage.is_open()) {
			m_hFDImage.close();
		}
	}

	bool OpenFDImage( const std::string &pFileName );
	bool CloseFDImage( void );
	virtual void ShowFDInfo( bool fShowTrackOffset );
	virtual FDC_STATUS	ReadSector( const int C, const int H, const int R, CSector &sect, size_t &numRead ) override;
	virtual FDC_STATUS	WriteSector( const int C, const int H, const int R, CSector &sect ) override;
	virtual bool isAvailable( void ) { return m_fAvailable; }
};

#endif
