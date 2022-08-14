#include <stdio.h>
#include <stdlib.h>

#define DATATYPE	(2)
#define ENDTYPE		(8)

/*
 *	16Bit	1	9
 *	24Bit	2	8
 *	32Bit	3	7
 */

typedef struct {
	unsigned char	Type;
	unsigned char	Size;
	unsigned long	Address;
	unsigned char	Data[128];
	unsigned char	Sum;
} SFORM;

SFORM Sdata;

/* SFORMの中身を空にする */
void Flush( SFORM *data )
{
	data->Type=0;
	data->Size=0;
	data->Address=0L;
	data->Sum=0;
}

/* レコードタイプをセットする */
void SetType( unsigned char type , SFORM *data )
{
	data->Type=type;
}

/* レコード開始アドレスをセットする */
void SetAddress( unsigned long address , SFORM *data )
{
	data->Address=address;
}

/* レコードにデータをストックする */
void Stock( unsigned char stkdata , SFORM *data )
{
	data->Data[data->Size++]=stkdata;
	data->Sum+=stkdata;
}

/* レコードを出力する */
void Put( SFORM *data )
{
	int i;
	unsigned long adr;
	unsigned char siz;
	fprintf(stdout, "S%d", data->Type);
	switch(data->Type) {
		case 0:
		case 1:
		case 9:	siz=3+data->Size;
				adr=data->Address;
				fprintf(stdout, "%02X%04LX", siz, adr);
				data->Sum+=siz;
				data->Sum+=(unsigned char)((adr >>8) & 0x00ffu);
				data->Sum+=(unsigned char) (adr		 & 0x00ffu);
				break;
		case 2:
		case 8:	siz=4+data->Size;
				adr=data->Address;
				fprintf(stdout, "%02X%06LX", siz, adr);
				data->Sum+=siz;
				data->Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				data->Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				data->Sum+=(unsigned char) (adr		  & 0x00ffu);
				break;
		case 3:
		case 7:	siz=5+data->Size;
				adr=data->Address;
				fprintf(stdout, "%02X%08LX", siz, adr);
				data->Sum+=siz;
				data->Sum+=(unsigned char)((adr >>24) & 0x00ffu);
				data->Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				data->Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				data->Sum+=(unsigned char) (adr		  & 0x00ffu);
				break;
	}
	for(i=0; i<data->Size; i++ ) {
		fprintf(stdout, "%02X", data->Data[i]);
	}
	fprintf(stdout, "%02X\n", (unsigned char)(~(data->Sum)));		/* データ合計の補数がチェックサム */
}

FILE *fopen_( char *filename , char *type )
{
	FILE *fp;
	if((fp=fopen(filename, type))==(FILE*)NULL) {
		printf("I couldn't open file '%s' in mode '%s'.\n", filename, type);
		exit(1);
	}
	return fp;
}

void main( int argc , char *argv[] )
{
	FILE *fpi;
	SFORM data;
	char *ptr;
	int ch, count;
	unsigned long adrs;

	if(argc!=2) {
		puts("MEMORY IMAGE FILE TO MOTOROLA-S FORMAT CONVERTER" );
		puts("COPYRIGHT MCMXCVII CANON SB450 DEVEROPMENT TEAM. ALL RIGHTS RESERVED.");
		printf("\nUsage : %s MEMORY_IMAGE_FILENAME\n\n", argv[0]);
		puts("Converted contents will be outputed to standard output equipment.");
		exit(-1);
	}

	fpi=fopen_(argv[1], "rb");

	Flush(&data);
	SetType(0, &data);			/* ヘッダレコード */
	SetAddress(0L, &data);
	ptr=argv[1];
	while(*ptr!=(char)NULL) {	/* ヘッダレコードの中身はファイル名 */
		Stock(*ptr++, &data);
	}
	Put(&data);

	adrs=0L;
	count=0;
	Flush(&data);
	SetAddress(adrs, &data);	/* 次のレコードの開始アドレス */
	SetType(DATATYPE, &data);	/* データレコードタイプを指定 */
	while(!feof(fpi)) {
		ch=fgetc(fpi);			/* ファイルから１バイト読み込み */
		if(ch==-1) break;
		Stock((unsigned char)ch, &data);		/* レコードバッファに格納 */
		count++;				/* レコードバッファカウンタを増やす */
		adrs++;					/* アドレスカウンタを増やす */
		if(count>=0x10) {		/* レコードバッファカウンタが0x10になったら、そこまでのデータを吐き出す */
			Put(&data);			/* レコード出力 */
			Flush(&data);		/* レコードバッファをフラッシュする */
			SetType(DATATYPE,&data);	/* データレコードタイプを指定 */
			SetAddress(adrs, &data);	/* 次のレコードのアドレス設定 */
			count=0;			/* レコードバッファカウンタをクリア */
		}
	}
	if(count)	Put(&data);		/* レコードバッファに、未送出データが残っている場合、それを出力する */

	Flush(&data);				/* 最後にエンドレコードを出力する */
	SetAddress(0L, &data);
	SetType(ENDTYPE, &data);	/* エンドレコードタイプ */
	Put(&data);

	fclose(fpi);
}
