#ifndef _CDUMP_H_
#define _CDUMP_H_

#include <iostream>

class CDump {
protected:
	uint8_t ascii[17];
	size_t readptr;
	size_t bytecount;
	void dispadrs( void );
	void dispascii( void );
public:
	CDump(void) { readptr=0UL; bytecount=0; }
	void put( uint8_t ch );
	void put( uint16_t ch );
	void put( uint32_t ch );
	void flush( void );
};

#endif // _CDUMP_H_
