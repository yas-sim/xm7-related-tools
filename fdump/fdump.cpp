#include <stdio.h>
#include <stdlib.h>

class Dump {
protected:
	unsigned char ascii[17];
	unsigned long readptr;
	int bytecount;
	void dispadrs( void );
	void dispascii( void );
public:
	Dump(void) { readptr=0UL; bytecount=0; }
	void put( unsigned char ch );
	void put( unsigned short ch );
	void put( unsigned long ch );
	void flush( void );
};

void Dump::dispadrs( void )
{
	printf("%08lX ", readptr);
}

void Dump::dispascii( void )
{
	if(bytecount==0) return;
	ascii[bytecount] = (unsigned char)NULL;
	for(; bytecount<0x10; bytecount++) {
		printf("   ");
	}
	puts((const char*)ascii);
	bytecount=0;
}

void Dump::flush( void )
{
	dispascii();
}

void Dump::put( unsigned char ch )
{
	
	if(ch<' ') 	ascii[readptr & 0x0000000fu] = '.';
	else		ascii[readptr & 0x0000000fu] = ch;

	if((readptr & 0x000000fful)==0x00)
		puts("\n-OFFSET- +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");
	if((readptr & 0x0000000ful)==0x00) dispadrs();
	printf("%02X ", ch);
	readptr++;
	bytecount++;
	if((readptr & 0x0000000ful)==0x00) dispascii();
}

void Dump::put( unsigned short ch )
{
	put((unsigned char)((ch>>8) & 0x00fful));
	put((unsigned char)((ch   ) & 0x00fful));
}

void Dump::put( unsigned long ch )
{
	put((unsigned char)((ch>>24) & 0x00fful));
	put((unsigned char)((ch>>16) & 0x00fful));
	put((unsigned char)((ch>> 8) & 0x00fful));
	put((unsigned char)((ch    ) & 0x00fful));
}

int main(int argc, char *argv[])
{
	FILE *fp;
	Dump dump;
	unsigned char ch;

	if(argc!=2) {
		puts("Usage : fdump dump_file[ret]");
		exit(1);
	}

	if((fp=fopen(argv[1], "rb"))==NULL) {
		perror("Input file open ");
		exit(2);
	}

	while(ch=(unsigned char)fgetc(fp), !feof(fp)) {
		dump.put((unsigned char)ch);
	}
	dump.flush();
	return 0;
}
