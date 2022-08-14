#ifndef _MOTGEN_H_
#define _MOTGEN_H_

#define motHEADER	(0)
#define motDATA16	(1)
#define motDATA24	(2)
#define motDATA32	(3)
#define motEND32	(7)
#define motEND24	(8)
#define motEND16	(9)

#define motUNIT		(0x10)		/* モトローラＳレコード長 */

typedef struct {
	unsigned char	Type;
	unsigned char	Size;
	unsigned long	Address;
	unsigned char	Data[32];
	unsigned char	Sum;
} SFORM;

extern void motInit( void );
extern void motSetType( unsigned char type );
extern void motSetAddress( unsigned long address  );
extern void motFlush( void );
extern void motStock( unsigned char stkdata );

#endif
