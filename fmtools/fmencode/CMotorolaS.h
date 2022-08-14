#ifndef _MOTOROLAS_H_
#define _MOTOROLAS_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_MOTOROLAS_LENGTH		(256)
#define MAX_MOTOROLAS_LINE_LENGTH	(2+2+8+(MAX_MOTOROLAS_LENGTH)*2+2)

class CMotorolaS {
private: //-----------------------------------------------------------------
	bool			m_fAvailable;		// 初期化に成功するとtrue
	int				m_nRecordType;		// レコードタイプ
	int				m_nByteCount;		// バイトカウント
	unsigned long	m_nLoadAddress;		// ロードアドレス
	unsigned char	*m_pData;			// データ
	unsigned char	m_nCheckSum;		// チェックサム

	int				m_nDataCount;		// データ数
	char			*m_pLineBuff;		// Sレコードラインバッファ

private: //-----------------------------------------------------------------
	inline unsigned char Hex2Dec(unsigned char ch)	{
		return (ch<='9')?(ch-'0'):((ch & ~0x20)-'A'+10);
	}

	inline unsigned char GetHex(char *pLine, int pos) {
		return (Hex2Dec(pLine[pos])<<4) | (Hex2Dec(pLine[pos+1]));
	}

	// レコードタイプからレコード長を返す
	int GetAddressLength( int nRecordType )
	{
		static int alen[11] = { 2,2,3,4,2,2,2,2,2,2,2 };	// 各レコードタイプごとのアドレス長
		if(!CheckRecordType(nRecordType)) return 0;			// 不正なレコードタイプの場合
		return alen[nRecordType];
	}

	// レコードタイプとして正しい番号かどうか調べてtrue/falseを返す
	bool CheckRecordType( int nRecordType )
	{
		if(0<=nRecordType && nRecordType<=9) return true;
		return false;
	}

	// pLineの最後に１文字追加する
	void AddChar( char *pLine, unsigned char ch )
	{
		if(strlen(pLine)+1<MAX_MOTOROLAS_LENGTH-1) {
			char tmp[2];
			tmp[0] = ch;
			tmp[1] = NULL;
			strcat(pLine, tmp);
		}
	}

	// pLineの最後に16進数を追加する
	void AddHex( char *pLine, unsigned char data )
	{
		if(strlen(pLine)+2<MAX_MOTOROLAS_LENGTH-1) {
			char tmp[3];
			sprintf(tmp, "%02X", data);
			strcat(pLine, tmp);
		}
	}

	// メンバーの設定値からチェックサムを計算する
	// ラインバッファの値(m_pLineBuff)は使用しない
	unsigned char CalcCheckSum( void );
	void GenerateSRecord( char *pLine );
	bool AnalyzeSRecord( char *pLine );

public: //-----------------------------------------------------------------
	CMotorolaS() {
		m_fAvailable = false;
		m_nRecordType = 0;
		m_nByteCount = 0;
		m_nDataCount = 2 + 0 + 1;	// アドレス長＋データ長＋チェックサム長
		m_nLoadAddress = 0;
		m_pData = new unsigned char[MAX_MOTOROLAS_LENGTH];
		if(m_pData==NULL) return;			// データ領域を確保できなかったので使用不可
		m_pLineBuff = new char[MAX_MOTOROLAS_LINE_LENGTH+1];
		if(m_pLineBuff==NULL) return;		// データ領域を確保できなかったので使用不可
		m_fAvailable = true;
		m_nCheckSum = CalcCheckSum();		// チェックサムを計算させる
		GenerateSRecord(m_pLineBuff);		// Sレコードを生成する
	}

	virtual ~CMotorolaS() {
		if(m_pData!=NULL) delete []m_pData;
		if(m_pLineBuff!=NULL) delete []m_pLineBuff;
	}

	int GetRecordType( void )				{ return m_fAvailable	?m_nRecordType	:-1; }
	int GetByteCount( void )				{ return m_fAvailable	?m_nByteCount	:-1; }
	int GetDataCount( void )				{ return m_fAvailable	?m_nDataCount	:-1; }
	unsigned long GetLoadAddress( void )	{ return m_fAvailable	?m_nLoadAddress	:0xffffffff; }
	unsigned char GetCheckSum( void )		{ return m_fAvailable	?m_nCheckSum	:0xff; }
	unsigned char GetData( int nPos );
	void SetRecordType( int nType );
	void SetByteCount( int nCount );
	void SetDataCount( int nCount );
	void SetLoadAddress( unsigned long nAddress );
	void SetData( int nPos, unsigned char nData );
	void SetDataBuff( int n, unsigned char *pData );
	void SetSRecord( char *pLine );
	unsigned char *GetDataBuff( unsigned char *pBuff );
	char *GetSRecordBuff( char *pBuff );
	void InitSRecord( unsigned long nAddress );
	void AddData( unsigned char data );
};

#endif // _MOTOROLAS_H_
