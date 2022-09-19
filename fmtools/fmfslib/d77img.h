
// D88/D77 Disk format
//	Header
//	Ofst		Size	Contents			Note
//	0x00		17		Disk name			(ended with '\0')
//	0x11		9		Reserved
//	0x1a		1		Write protect		0x00=No-protect	0x10=Protected
//	0x1b		1		Disk type			0x00:2D, 0x10:2DD, 0x20:2HD, 0x30:1D, 0x40:1DD
//	0x1c		4		Disk size			Head + all tracks
//	0x20		4*164	Track offset table	Offset to the track data from the top of the file
//
// Sector
//	Ofst		Size	Contents			Note
//	0x00		1		Cylinder#
//	0x01		1		Head#
//	0x02		1		Sector#
//	0x03		1		Secor size code
//	0x04		2		# of sectors in the track		(128<<N)
//	0x06		1		Density				0x00:Double density, 0x40:Single density, 0x01:High density
//	0x07		1		DAM					0x00:DAM, 0x10:DDAM
//	0x08		1		Status				0x00:Normal, 0x10:DDAM, 0xa0:ID-CRC, 0xb0:DT-CRC, 0xe0:No IDAM, 0xf0:No DAM
//	0x09		5		Reserved
//	0x0e		2		Sector data size
//	0x10		#		Sector data

#include <ifstream>

int main(void) {
	
}