#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <locale>

#define VERSION	"0.1"

#include "cmotorolas.h"

// 文字が１６進数かどうか調べる
bool ishex( char ch ) {
	switch(toupper(ch)) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		return true;
		break;
	}
	return false;
}

#define MAX_VAL		(128)

long g_val[MAX_VAL];			// ダンプの行を分解して、数値に直した値の配列(val[0]はアドレス)
int g_nov;						// g_valの要素の数

// １６進文字列を数値に変換
long hex2val( char *buff ) {
	char ch;
	long val = 0l;
	while(*buff) {
		ch = toupper(*buff++);
		if(ch<=' ') continue;							// 制御コード \r,\nなどは無視する
		if(ishex(ch)==false) return -1;					// 16進文字以外が含まれていたのでNG
		val = val*0x10 + ((ch<='9')?ch-'0':ch-'A'+10);	// 16進文字を数値に変換
	}
	return val;
}

// 与えられた文字列をトークンに分解した後、数値に変換して配列(g_val)に格納する
bool split( char *string ) {
	char tmpbuff[256];
	char *ptr;
	g_nov = 0;
	strncpy(tmpbuff, string, 255);
	ptr = strtok(tmpbuff, " :=\t");			// 最初のトークンを切り出す
	while(ptr!=nullptr) {
		if(g_nov>=MAX_VAL) break;			// 数値配列の要素数を超えるときは終了する
		g_val[g_nov++] = hex2val(ptr);		// 数値に変換
		ptr = strtok(NULL, " \t");			// 二つ目以降のトークンを切り出す
	}
	return true;
}

// g_valの最初をアドレス、最後の値をチェックサムと仮定してチェックサムを計算
bool check( void ) {
	int sum = 0;
	int i;
	for(i=1; i<g_nov-1; i++) {
		sum += g_val[i];
	}
	if((sum & 0x00ff)==g_val[i]) return true;	// チェックサムがあっているか?
	return false;
}

void title( void ) {
	puts("Dump file to Motorola-S converter v" VERSION);
}

void usage( void ) {
	puts("Synopsis: dmp2mot infile outfile [options]");
	puts("Option:");
	puts("  -s  : Check sum including mode");
	puts("        Treat the input file as dump file with check sum");
	puts("  -v  : Verbose mode");
	puts("        Show source string after conversion");
}

// 変換テーブル構造体
struct ctbl {
	char	ch;
	char	str[128];
	struct ctbl *next;
};

// 変換テーブルリストコントロールクラス
class CConvertTable {
	ctbl	*m_root;
public:
	CConvertTable() {
		m_root = NULL;
	}
	~CConvertTable() {
		ctbl *ptr = m_root;
		ctbl *nptr;
		while(ptr!=NULL) {
			nptr = ptr->next;
			delete ptr;
			ptr = nptr;
		}
	}
	// 最後のテーブルを検索する
	ctbl *GetLast( void ) {
		ctbl *ptr;
		ptr = m_root;
		if(ptr==NULL) return NULL;
		while(ptr->next!=NULL) ptr = ptr->next;
		return ptr;
	}
	ctbl *AddTbl( void ) {
		if(m_root==NULL) {
			m_root = new ctbl;
			m_root->next = NULL;
			return m_root;
		}
		ctbl *ptr;
		ptr = GetLast();
		ptr->next = new ctbl;
		ptr->next->next = NULL;
		ptr->next->str[0] = '\0';
		return ptr->next;
	}
	// 指定の番号のテーブルを返す
	ctbl *GetTbl( int n ) {
		if(n<0) return NULL;
		ctbl *ptr;
		ptr = m_root;
		for(;n>0;n--) {
			if(ptr->next==NULL) return NULL;
			ptr = ptr->next;
		}
		return ptr;
	}
};

CConvertTable cnvtbl;

// 文字変換テーブルの読み込み。'cnvtbl.txt'というファイルから読み込む
void ReadConvertTable( void ) {
	FILE *fp;
	ctbl *ptr;
	char buff[128];
	if((fp=fopen("cnvtbl.txt", "rt"))==nullptr) return;
	while(!feof(fp)) {
		fgets(buff, 127, fp);
		ptr = cnvtbl.AddTbl();
		ptr->ch = buff[0];						// 変換文字をコピー
		_tcsncpy(ptr->str, buff+2, 127);		// 変換対象文字列をコピー
		for(char *tmp=ptr->str; *tmp; tmp++) 
			if((unsigned)*tmp<0x20) *tmp='\0';	// 変換対象文字絵列から制御コードを消し去る
	}
	fclose(fp);
}

// OCRにかけたダンプに含まれそうな誤変換を除去する
void ConvertString( char *string ) {
	ctbl *tblptr;
	char tmp_r[512];
	int n = 0;
	while((tblptr=cnvtbl.GetTbl(n++))!=NULL) {
		while(1) {
			int len = _tcslen(string);					// 文字列長を取得
			int ptr = _tcscspn(string, tblptr->str);	// 文字列中の、変換対象文字位置を検索
			if(ptr>=len) break;
			// 見つかった位置より右側のみをコピー
			_tcscpy(tmp_r, string+ptr+1);
			string[ptr] = tblptr->ch;			// 対象文字を置き換え
			_tcscpy(string+ptr+1, tmp_r);		// とっておいた対象文字より右側を連結
		}
	}
	// 全部大文字に変換しておく
	strupr(string);

	// ダンプリストを一度トークンに切り離し、再度合成する
	// データ部が2桁以上になっている場合、それを分離する(02AF→02 AF)
	char tmpbuff[256];					// strtokを使用するので、元文字列が破壊されないようにするバッファ
	char synth[256];					// 再合成するバッファ
	char space[2] = " ";
	char *ptr;							// strtok用、トークンポインタ
	int count = 0;						// トークンの数を数える
	synth[0] = '\0';					// 再合成バッファを空にする
	strncpy(tmpbuff, string, 255);		// テンポラリバッファに元文字列をコピーする
	ptr = strtok(tmpbuff, " \t");		// 最初のトークンを切り出す
	while(ptr!=NULL) {
		if(count==0) {
			strcpy(synth, ptr);			// 最初のトークンはアドレスなので、無条件コピー
		} else {
			// ２つめ以降のトークンは2桁以上ある場合は切り離す
			char *dstptr = synth + strlen(synth);	// 再合成バッファの終わりを取得
			int chn=0;						// 桁数カウンタ
			while(*ptr) {
				if(chn<2) {					// 2桁以下ならそのまま1文字コピー
					*dstptr++ = *ptr++;
					chn++;
				} else {
					*dstptr++ = ' ';		// 3桁目になったら、スペースをはさんでコピー
					*dstptr++ = *ptr++;
					chn=0;
				}
			}
			*dstptr = '\0';					// 再合成バッファのターミネータをつける
		}
		strcat(synth, space);				// 再合成バッファにスペースを追加
		count++;							// トークンカウンタを増やす
		ptr = strtok(NULL, " :=\t");		// 二つ目以降のトークンを切り出す
	}
	for(ptr=synth; *ptr; ptr++) 
		if((unsigned)*ptr<0x20) *ptr='\0';			// 変換対象文字絵列から制御コードを消し去る
	strcpy(string, synth);							// 再合成バッファで元文字列を上書き
}

int main( int argc, char *argv[]) {

	title();
	
	char infile[256], outfile[256];
	infile[0] = outfile[0] = '\0';
	bool fSUM = false;					// チェックサムをチェックするかどうか
	bool fVerbose = false;
	for(int i=1; i<argc; i++) {
		switch(argv[i][0]) {
		case '-':
		case '/':
			switch(argv[i][1]) {
			case 's':
			case 'S':
				fSUM = true;			// チェックサムをチェックする
				break;
			case 'v':
			case 'V':
				fVerbose = true;		// Verboseモード
				break;
			default:
				usage();
				exit(1);
			}
			// オプション指定子で始まるとき
			break;
		default:
			// オプション指定子で始まらないときは入出力ファイル名
			if(strlen(infile)==0) {
				strncpy(infile, argv[i], 255);		// 入力ファイル名
			} else {
				strncpy(outfile, argv[i], 255);		// 出力ファイル名
			}
			break;
		}
	}
	if(strlen(infile) ==0) strcpy(infile,  "test.txt");
	if(strlen(outfile)==0) strcpy(outfile, "test.mot");

	FILE *fpi, *fpo;
	// 入力ファイルオープン
	if((fpi=fopen(infile, "rt"))==NULL) {
		perror("Failed to open input file");
		exit(1);
	}
	// 出力ファイルオープン
	if((fpo=fopen(outfile, "wt"))==NULL) {
		fclose(fpi);
		perror("Failed to open output file");
		exit(1);
	}
	ReadConvertTable();		// 文字変換テーブル'convtbl.txt'を読み出す

	char buff[512];
	CMotorolaS mot;
	while(!feof(fpi)) {
		if(fgets(buff, 511, fpi)==NULL) break;	// 1行読み込み
		ConvertString(buff);					// 文字補正
		if(fVerbose) puts(buff);
		split(buff);							// 行を分解して、数値列に変換
		if(g_val[0]!=-1) {						// アドレスが有効か？
			if(fSUM==true) {					// チェックサムありモードか？
				if(check()==false) {
					fprintf(stderr, "Check sum error at $%04x\n", g_val[0]);
					continue;
				}
			}
			mot.InitSRecord(g_val[0]);			// アドレス値でSレコードを初期化
			// チェックサムがある場合、最後の数値はチェックサムなのでSレコード出力しない
			int maxv;
			if(fSUM==true) maxv=g_nov-1; else maxv=g_nov;

			for(int i=1; i<maxv; i++) {
				mot.AddData((unsigned char)g_val[i]);	// Sレコードにデータを追加
			}
			mot.GetSRecordBuff(buff);			// 変換したSレコードを取得
			fputs(buff, fpo);					// ファイルに出力
			fputs("\n", fpo);
		}
	}

	fclose(fpi);
	fclose(fpo);
	return 0;
}
