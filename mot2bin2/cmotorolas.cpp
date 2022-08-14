#include "cmotorolas.h"

// メンバーの設定値からチェックサムを計算する
// ラインバッファの値(m_pLineBuff)は使用しない
unsigned char CMotorolaS::CalcCheckSum( void )
{
	if(!m_fAvailable) return 0;				// 初期化失敗している
	m_nCheckSum = m_nByteCount;
	// アドレスを、レコードタイプ別に足し算する
	switch(m_nRecordType) {
	case 3:
	case 7:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >>24) & 0x00ffu);
		// breakなし
	case 2:
	case 8:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >>16) & 0x00ffu);
		// breakなし
	case 0:
	case 1:
	case 9:
		m_nCheckSum+=(unsigned char)((m_nLoadAddress >> 8) & 0x00ffu);
		m_nCheckSum+=(unsigned char) (m_nLoadAddress       & 0x00ffu);
		break;
	default:
		m_nCheckSum = 0;
		break;
	}
	// データをチェックサムに加算する
	for(int i=0; i<m_nDataCount; i++) {
		m_nCheckSum += m_pData[i];
	}
	m_nCheckSum = (unsigned char)(~m_nCheckSum);	// バイトカウント＋アドレス＋データの１の補数
	return m_nCheckSum;
}

// メンバ変数の値を使ってpLineにSレコードを生成する
void CMotorolaS::GenerateSRecord( char *pLine )
{
	int nCount = 0;					// Sレコードの文字数をカウントする。MAX_MOTOROLAS_LENGTHを超えないように管理
	if(!m_fAvailable) return;
	pLine[0] = '\0';
	AddChar(pLine, 'S');					// Sレコードのレコードヘッダ
	AddChar(pLine, m_nRecordType + '0');	// レコードタイプ
	AddHex(pLine, m_nByteCount);			// バイトカウント
	// アドレス出力
	switch(m_nRecordType) {
	case 3:
	case 7:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >> 24) & 0x00ff));
		// breakなし
	case 2:
	case 8:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >> 16) & 0x00ff));
		// breakなし
	case 0:
	case 1:
	case 9:
		AddHex(pLine, (unsigned char)((m_nLoadAddress >>  8) & 0x00ff));
		AddHex(pLine, (unsigned char)((m_nLoadAddress      ) & 0x00ff));
		break;
	default:
		break;
	}
	// データ出力
	for(int i=0; i<m_nDataCount; i++) {
		AddHex(pLine, m_pData[i]);
	}
	AddHex(pLine, m_nCheckSum);				// チェックサム
}

	// pLineをSレコードとして解析して、パラメータをメンバ変数に格納する
bool CMotorolaS::AnalyzeSRecord( char *pLine )
{
	int				nRecordType;		// レコードタイプ
	int				nByteCount;			// バイトカウント
	unsigned long	nLoadAddress;		// ロードアドレス
	unsigned char	nCheckSum;			// チェックサム
	int				nReadPtr;			// 解析位置
	int				nDataCount;
	unsigned char	ch;

	if(!m_fAvailable) return false;						// 初期化失敗している
	if(m_pLineBuff[0]!='S') return false;				// Sレコードではない
	nRecordType = Hex2Dec(pLine[1]);
	if(!CheckRecordType(nRecordType)) return false;		// レコードタイプが不正
	nByteCount = GetHex(pLine, 2);
	nDataCount = nByteCount - GetAddressLength(nRecordType) -1;	// データ長を算出
	if(nDataCount>MAX_MOTOROLAS_LENGTH) return false;	// Sレコードが長すぎる
	nCheckSum = nByteCount;								// チェックサムに足す
	// ロードアドレス取得
	nReadPtr = 4;
	nLoadAddress = 0L;
	for(int i=0; i<GetAddressLength(nRecordType); i++) {
		ch = GetHex(pLine, nReadPtr);
		nLoadAddress = (nLoadAddress<<8) + ch;
		nCheckSum += ch;								// チェックサムに足す
		nReadPtr += 2;
	}
	unsigned char *pTmpBuff = new unsigned char[nDataCount];	// 一時的データバッファを確保する
	if(pTmpBuff==NULL) return false;					// メモリが足りない…
	for(int i=0; i<nDataCount; i++) {
		ch = GetHex(pLine, nReadPtr);
		pTmpBuff[i] = ch;
		nCheckSum += ch;
		nReadPtr += 2;
	}
	// チェックサムの比較
	nCheckSum = (unsigned char)(~nCheckSum);
	if(nCheckSum != GetHex(pLine, nReadPtr)) {
		delete []pTmpBuff;					// きちんと一時データバッファを開放してからリターン
		return false;
	}
	// チェックサムが確認できたので、メンバ変数に解析結果をコピーする
	m_nRecordType = nRecordType;
	SetDataCount(nDataCount);	// m_nRecordTypeの値とデータ数からm_nByteCountも自動設定
	m_nLoadAddress = nLoadAddress;
	for(int i=0; i<nDataCount; i++) {
		m_pData[i] = pTmpBuff[i];
	}
	m_nCheckSum = nCheckSum;
	delete []pTmpBuff;
	return true;
}

unsigned char CMotorolaS::GetData( int nPos )
{
	if(!m_fAvailable) return 0;				// 初期化失敗している
	if(nPos>=MAX_MOTOROLAS_LENGTH) return 0;
	return m_pData[nPos];
}

// Sレコードデータのレコードタイプを設定する
void CMotorolaS::SetRecordType( int nType )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	if(!CheckRecordType(nType)) return;
	m_nRecordType = nType;
	m_nByteCount = m_nDataCount + GetAddressLength(nType) + 1;		// 1はチェックサムの分
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// Sレコードデータのバイトカウントを設定する
void CMotorolaS::SetByteCount( int nCount )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	if(nCount - GetAddressLength(m_nRecordType) - 1 >= MAX_MOTOROLAS_LENGTH) return;
	m_nByteCount = nCount;
	m_nDataCount = m_nByteCount - GetAddressLength(m_nRecordType) -1;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// Sレコードデータのデータ数を設定する
void CMotorolaS::SetDataCount( int nCount )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	if(nCount>=MAX_MOTOROLAS_LENGTH) return;
	m_nByteCount = nCount + GetAddressLength(m_nRecordType) + 1;	// 1はチェックサムの分
	m_nDataCount = nCount;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// Sレコードデータのロードアドレスを設定する
void CMotorolaS::SetLoadAddress( unsigned long nAddress )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	m_nLoadAddress = nAddress;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// Sレコードデータの任意の位置にデータを書き込む
void CMotorolaS::SetData( int nPos, unsigned char nData )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	if(nPos>=MAX_MOTOROLAS_LENGTH) return;
	if(nPos>=m_nDataCount) SetDataCount(nPos+1);		// 今まで書き込んだ最大データより大きい位置に書き込もうとしたため、バイトカウントを増やす
	m_pData[nPos] = nData;
	m_nCheckSum = CalcCheckSum();
	GenerateSRecord(m_pLineBuff);
}

// 指定のデータバッファをSレコードのデータバッファにコピーする。バイトカウントも自動設定する
// それ以前にデータバッファに書かれていた内容は無効
void CMotorolaS::SetDataBuff( int n, unsigned char *pData )
{
	unsigned char *tmp;
	if(!m_fAvailable) return;				// 初期化失敗している
	if(n>=MAX_MOTOROLAS_LENGTH) return;
	tmp = m_pData;
	for(int i=0; i<n; i++) {
		*tmp++ = *pData++;
	}
	SetByteCount(n);
//	GenerateSRecord(m_pLineBuff);		// SetByteCount()がGenerateSRecord()を含んでいるので不要
}

// 指定のアスキー文字列をSレコード文字列として格納し、解析する。解析結果はメンバ変数に格納する。
void CMotorolaS::SetSRecord( char *pLine )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	strncpy(m_pLineBuff, pLine, MAX_MOTOROLAS_LINE_LENGTH);
	if(!AnalyzeSRecord(m_pLineBuff)) {
		InitSRecord(0);					// Sレコードを解析した結果異常なレコードであることが分かったので、オブジェクトを初期化する
	}
}

// データバッファのデータをコピーし、その先頭アドレスを返す
// 与えられたコピー先のアドレスはデータコピーに十分なサイズが確保されている必要あり
unsigned char *CMotorolaS::GetDataBuff( unsigned char *pBuff )
{
	unsigned char *src, *dst;
	if(!m_fAvailable) return 0;				// 初期化失敗している
	src = m_pData;
	dst = pBuff;
	for(int i=0; i<m_nDataCount; i++) {
		*dst++ = *src++;
	}
	return pBuff;
}

// Sレコードバッファのデータをコピーし、その先頭アドレスを返す
// 与えられたコピー先のアドレスはデータコピーに十分なサイズが確保されている必要あり
char *CMotorolaS::GetSRecordBuff( char *pBuff )
{
	if(!m_fAvailable) return 0;				// 初期化失敗している
	strcpy(pBuff, m_pLineBuff);
	return pBuff;
}

// 指定のアドレスから始まるようにSレコードデータを初期化する
// アドレス値をチェックして、適切なレコードタイプを選択する
void CMotorolaS::InitSRecord( unsigned long nAddress )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	m_nRecordType = 1;
	if(nAddress+MAX_MOTOROLAS_LENGTH > 0x00010000) m_nRecordType = 2;
	if(nAddress+MAX_MOTOROLAS_LENGTH > 0x01000000) m_nRecordType = 3;
	SetDataCount(0);
	m_nLoadAddress = nAddress;
	m_nCheckSum = CalcCheckSum();
}

// Sレコードのデータの最後にデータを追加する
void CMotorolaS::AddData( unsigned char data )
{
	if(!m_fAvailable) return;				// 初期化失敗している
	if(m_nDataCount>=MAX_MOTOROLAS_LENGTH) return;
	SetData(m_nDataCount, data);
}
