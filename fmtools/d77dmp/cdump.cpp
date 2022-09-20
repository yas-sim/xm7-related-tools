#include <stdio.h>
#include <stdlib.h>

#include "cdump.h"

void CDump::dispadrs( void )
{
	printf("%08lX ", readptr);
}

void CDump::dispascii( void )
{
	if(bytecount==0) return;
	ascii[bytecount] = (unsigned char)NULL;
	for(; bytecount<0x10; bytecount++) {
		printf("   ");
	}
	puts((const char*)ascii);
	bytecount=0;
}

void CDump::flush( void )
{
	dispascii();
}

void CDump::put( uint8_t ch )
{
	if(ch<' ' || ch>=0x7f) 	ascii[readptr & 0x0fu] = '.';
	else					ascii[readptr & 0x0fu] = ch;

	if((readptr & 0x00fful)==0x00)
		puts("\n-OFFSET- +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");
	if((readptr & 0x000ful)==0x00) dispadrs();
	printf("%02X ", ch);
	readptr++;
	bytecount++;
	if((readptr & 0x0ful)==0x00) dispascii();
}

void CDump::put( uint16_t ch )
{
	put((uint8_t)((ch>>8) & 0x00fful));
	put((uint8_t)((ch   ) & 0x00fful));
}

void CDump::put( uint32_t ch )
{
	put((uint8_t)((ch>>24) & 0x00fful));
	put((uint8_t)((ch>>16) & 0x00fful));
	put((uint8_t)((ch>> 8) & 0x00fful));
	put((uint8_t)((ch    ) & 0x00fful));
}
