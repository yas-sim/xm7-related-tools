#include <stdio.h>
#include <stdlib.h>

#include "motorolas.h"



static SFORM motDATA;





#define HEX2DEC(CH)				(((CH)<='9')?((CH)-'0'):((CH)-'A'+10))
#define GETHEX(BUFFER, POS)		((HEX2DEC(BUFFER[(POS)])<<4) | (HEX2DEC(BUFFER[(POS)+1])))

const int MAXSRECORDSIZE=256;

class CMotorolaS {
private:
	int				m_nSum;					// 現在のチェックサム
	unsigned long	m_nAdrs;				// 現在のトップアドレスとボトムアドレス
	int				m_nRecordType;			// レコードタイプ
	int				m_nLengh;				// データ部のバイト数
	bool			m_fValid;				// 現在、有効なレコードが読み込まれているかどうか
	int				m_nLineCount;			// 読み込んだ行数
	int				m_nAdrLen;
	char			m_cpBuff[MAXSRECORDSIZE];
	char			m_cpData[MAXSRECORDSIZE/2];
	FILE			*m_fp;
	
	void _getAdrLen( void )
	{
		int alen[11] = { 2,2,3,4,2,2,2,2,2,2,2 };	// 各レコードタイプごとのアドレス長
		m_nAdrLen = alen[_getRecordType()];	
	}
	
	void _getData( char *databuff )
	{
		int i, byte, ofst;
		byte = _getByteCount()-1;
		ofst = m_nAdrLen;
		for(i=ofst; i<byte; i++) {
			*databuff++ = GETHEX(m_cpBuff, i*2+4);
		}
	}

	unsigned long _getAddress( void )
	{
		unsigned long result;
		result = 0ul;

		for(int i=0; i<m_nAdrLen; i++) {
			result = (result << 8) | GETHEX(m_cpBuff, i*2+4);
		}
		return result;
	}

	// レコードからチェックサムを抜き出す
	int _getSum( void )			{ return GETHEX(m_cpBuff, _getByteCount()*2+2);	}
	int _getRecordType( void )	{ return m_cpBuff[1] & 0x0f;	}
	int _getByteCount( void )	{ return GETHEX(m_cpBuff, 2);	}

public:

	CMotorolaS()
	{
		m_fp			= (FILE*)NULL;
		m_cpBuff[0]		= '\0';
		m_nSum			= -1;
		m_nAdrs			= 0UL;
		m_nRecordType	= -1;
		m_fValid		= false;
		m_nLineCount	= 0;
	}

	CMotorolaS( char *filename , char *mode )
	{
		open(filename, mode);
		CMotorolaS();
	}

	virtual ~CMotorolaS( void )
	{
		close();
	}

	// ファイルを開く
	void open( char *filename , char *mode )
	{
		m_fp = fopen(filename, mode);
	}
	
	// ファイルを閉じる
	void close( void )
	{
		if(m_fp!=(FILE*)NULL) fclose(m_fp);
		m_fp = (FILE*)NULL;
	}

	// 行（キャラクタ）データからチェックサムを計算する
	int calcSum( void )
	{
		int sum;
		sum = 0;
		if(isValid()) {
			for(int i=0; i<_getByteCount(); i++) {
				sum+= GETHEX(m_cpBuff, i*2+2);
			}
			return (~sum) & 0x00ff;
		}
		return -2;		// getSum()がエラー時に-1を返すので、calsSum()もエラー時に-1を返すとチェックサムが等しいと勘違いされるのを防ぐ目的で-2を返す
	}

	// 現在の行バッファ(m_cpBuff)を解析して、クラスメンバ各種パラメータを設定する
	void analyzeRecord( void )
	{
		m_nRecordType	= _getRecordType();					// レコードタイプ
		_getAdrLen();										// アドレス部の長さをadrslenに格納する
		m_nLengh		= _getByteCount()-m_nAdrLen-1;		// データ部のみのデータ長
		m_nAdrs			= _getAddress();					// レコードのアドレス
		_getData(m_cpData);									// Sレコードからデータを抜き出す
		m_nSum			= _getSum();						// Sレコードに書かれているチェックサム値
	}

	// ファイルから１レコード読み込み、クラスメンバに各パラメータを設定する
	bool getRecord( void )
	{
		m_fValid = false;
		if(m_fp!=NULL) {
			do {
				fgets(m_cpBuff, MAXSRECORDSIZE-2, m_fp);
				m_nLineCount ++;									// 読み込んだ行数を１増やす
			} while(m_cpBuff[0]!='S' && m_cpBuff[1]!='s' && !feof(m_fp));	// Sレコード以外の行を読み飛ばす
			if(feof(m_fp)) {
				m_nLineCount = 0;
				m_fValid = false;
				return false;
			}
			m_fValid = true;
			analyzeRecord();
			return true;
		} else {
			m_fValid = false;
			return false;
		}
	}

	// バイトカウント、ロードアドレス、データからチェックサムを求める
	unsigned char calcTotalSum( void )
	{
		
	}

	void putchToBuff( char ch )
	{
	}

	void generateSRecord( void )
	{
		unsigned char *ptr;

		ptr = (unsigned char*)m_cpBuff;


	}

	// Sレコード生成時に使用するデータレコードタイプ
	void setRecordType( int type ) {
		m_nRecordType = type;
	}

	int isValid( void )				{ return m_fValid; }
	int getLength( void )			{ return isValid() ? m_nLengh			: -1;			}
	int getLineCount( void )		{ return isValid() ? m_nLineCount		: -1;			}
	int getRecordType( void )		{ return isValid() ? m_nRecordType		: -1;			}
	int getSum( void )				{ return isValid() ? m_nSum				: -1;			}
	unsigned long getAdrs( void )	{ return isValid() ? m_nAdrs			: 0xfffffffful; }
	int getCalcSum( void )			{ return isValid() ? calcSum()			: -1;			}
	bool isFileOpen( void )			{ return (m_fp!=(FILE*)NULL) ? true		: false;		}
	char *getBuff( void )			{ return isValid() ? m_cpBuff           : "Invalid Line\n\0"; }
};




//-----------------------------------------------------------------------------------

/* SFORMの中身を空にする */
void motInit( void )
{
	motDATA.Type=0;
	motDATA.Size=0;
	motDATA.Address=0L;
	motDATA.Sum=0;
}

/* レコードタイプをセットする */
void motSetType( unsigned char type )
{
	motDATA.Type=type;
}

/* レコード開始アドレスをセットする */
void motSetAddress( unsigned long address  )
{
	motDATA.Address=address;
}

/* レコードを出力する */
static void motPut( void )
{
	int i;
	unsigned long adr;
	unsigned char siz;

	fprintf(stdout, "S%d", motDATA.Type);
	switch(motDATA.Type) {
		case 0:
		case 1:
		case 9:	siz=3+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%04LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr	   & 0x00ffu);
				break;
		case 2:
		case 8:	siz=4+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%06LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
		case 3:
		case 7:	siz=5+motDATA.Size;
				adr=motDATA.Address;
				fprintf(stdout, "%02X%08LX", siz, adr);
				motDATA.Sum+=siz;
				motDATA.Sum+=(unsigned char)((adr >>24) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >>16) & 0x00ffu);
				motDATA.Sum+=(unsigned char)((adr >> 8) & 0x00ffu);
				motDATA.Sum+=(unsigned char) (adr		& 0x00ffu);
				break;
	}
	for(i=0; i<motDATA.Size; i++ ) {
		fprintf(stdout, "%02X", motDATA.Data[i]);
	}
	fprintf(stdout, "%02X\n", (unsigned char)(~(motDATA.Sum)));		/* データ合計の補数がチェックサム */
}

/* 溜まっているデータを出力し、バッファをフラッシュする */
void motFlush( void )
{
	motPut();
	motDATA.Address+= motDATA.Size;
	motDATA.Size=0;
	motDATA.Sum=0;
}

/* レコードにデータをストックする */
/* データがレコード長になったら、それまでのデータを出力する */
void motStock( unsigned char stkdata )
{
	motDATA.Data[motDATA.Size++]=stkdata;
	motDATA.Sum+=stkdata;
	if(motDATA.Size>=motUNIT) {
		motFlush();
	}
}