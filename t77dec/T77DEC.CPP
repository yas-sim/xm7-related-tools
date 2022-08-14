#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>

// v0.3 001004  ビット判定方法を、T77→ビットストリーム化→判定に変更。処理時間はかかるが、正確
// v0.4 001005	BASIC(I-CODE)=IBA, (ASCII)=BASに拡張子を変更
// v0.5 001009  マシン語ファイルを受信時、１バイト余分にデータを書き込んでいたのを修正
// V0.6 010114	マシン語受信時、ヘッダの読みだし開始位置を5増やしているのに、さらにブロックサイズを5引いてしまっていたバグを修正

#define VERSION		"0.6"

#define T77RESOLUTION	(9e-6)

#define ZERO_FREQ	(2400)
#define ONE_FREQ	(1200)

#define ZERO_VAL	(0x17)
#define ONE_VAL		(0x2e)

inline int GetBitStatus( FILE *fp, bool phase ) {
	static long period = 0L;
	static int sign = 0;
	int data1, data2;
	int data;

	while(period<=0) {
		if((data1=fgetc(fp))==EOF) return -1;
		if((data2=fgetc(fp))==EOF) return -1;
		data = (data1<<8) | data2;
		period += (data & 0x7fff)*9;
		sign = (data & 0x8000)==0x8000?1:0;
	}
	period--;	
	return phase?sign:1-sign;	
}

int GetBit(FILE *fp, bool phase) {
	int bit;
	// 立ち上がり検出
	do {
		bit = GetBitStatus(fp, phase);
		if(bit==-1) return -1;
	} while(bit!=0);
	do {
		bit = GetBitStatus(fp, phase);
		if(bit==-1) return -1;
	} while(bit==0);
	int check_point = 312;		// 2400bpsでの3/4周期ポイント
	for(int i=0; i<check_point; i++) {
		bit = GetBitStatus(fp, phase);
	}
	return bit;
}

int GetByte(FILE *fp, bool phase) {
	int bit, byte;
	int stop1, stop2;
	byte = 0;
	// スタートビット待ち
	do {
		bit=GetBit(fp, phase);
		if(bit==-1) return -1;
	} while(bit!=0);
	// データフレーム構築
	for(int i=0; i<8; i++) {
		bit=GetBit(fp, phase);
		if(bit==-1) return -1;
		byte = (bit==0?0:0x80) | (byte>>1);
	}
	// ストップビットチェック
	stop1=GetBit(fp, phase);
	if(stop1==-1) return -1;
	stop2=GetBit(fp, phase);
	if(stop2==-1) return -1;
	if(stop1!=1 || stop2!=1) return -2;	// フレーミングエラー
	return byte;
}

int GetBlock( FILE *fp, bool phase, unsigned char *buff, int &type) {
	int ch;
	int previous = 0;
	int len = 0;
	// ヘッダマーカー(0x013c)待ち
	while(1) {
		ch = GetByte(fp, phase);
		if(ch==-1) return -1;
		if(previous==0x01 && ch==0x3c) break;
		previous = ch;
	}
	int blen, sum;
	if((type = GetByte(fp, phase))==-1) return -1;
	if((blen = GetByte(fp, phase))==-1) return -1;
	sum = type + blen;			// チェックサム初期値
	for(int i=0; i<blen; i++) {
		ch = GetByte(fp, phase);
		if(ch==-1) return -1;
		len++;
		*buff++ = ch;
		sum += ch;
	}
	int expsum = GetByte(fp, phase);
	if(expsum==-1) return -1;
	if(expsum!=(sum & 0xff)) return -3;
	return len;
}


void usage( void )
{
	puts("T77 file to binary file converter for XM7 " VERSION "\n");
	puts("Synopsis: t77dec infile [options]\n");
	puts("Option : -v       Verbose mode");
	puts("         -p[mode] Specify phase");
	puts("                  mode: r=Reverse, n=Normal[default]");
	puts("         -d       full Dump mode(for debug)");
	puts("         -t[val]  Bit recognize torelance[%]. Default is 50%");
	puts("         -h       Show this message");
	puts("Output filename will be generated from original filename");
	puts("ex1. t77dec test.t77 ");
	puts("ex2. t77dec test.t77 -v -pr -t60");
}

void main(int argc, char *argv[])
{
	FILE *fpi, *fpo;

	if(argc<2) {
		usage();
		exit(0);
	}

	// オプション解析
	int source=0;
	bool fVerbose = false;
	bool fPhase=true;		// 音声ファイルの位相(true=Normal False=Reverse)
	bool fDump = false;		// ヘッダなどすべてのデータをデコードする(true)か、データのみでコードするか(false)
	for(int i=1; i<argc; i++) {
		if(argv[i][0]=='-' || argv[i][0]=='/') {
			switch(argv[i][1]) {
			case 'p':
			case 'P':					// 位相の指定
				switch(argv[i][2]) {
				case 'n':
				case 'N':
					fPhase = true;		// 正相
					break;
				case 'r':
				case 'R':
					fPhase = false;		// 逆相
					break;
				}
				break;
			case 'h':
			case 'H':
			case '?':
				usage();
				exit(0);
				break;
			case 'v':
			case 'V':
				// Verbose modeセット
				fVerbose = true;
				break;
			case 'd':
			case 'D':
				// Full dump モード
				fDump = true;
				break;
			default:
				break;
			}
		} else {
			// オプション指定子がない場合、ファイル名とみなす
			source=i;
		}
	}

	if(fVerbose) {
		puts("T77 file decoder version " VERSION);
		puts("Programmed by an XM7 supporter");
		printf("Phase = %s\n", fPhase?"Normal":"Reverse");
	}

	if((fpi=fopen(argv[source], "rb"))==NULL) {
		perror("Source file open");
		exit(2);
	}

	unsigned char buff[1024];
	// ヘッダーチェック!
	fgets((char*)buff, 20, fpi);
	if(strcmp((char*)buff, "XM7 TAPE IMAGE 0")!=0) {
		puts("Source file is not a valid XM7 tape image file");
		exit(1);
	}
	fseek(fpi, 0x10, SEEK_SET);		// ヘッダ読み飛ばし

	if(fDump==true) {
		// Full Dump
		if((fpo=fopen("FULLDUMP.BIN", "wb"))==NULL) {
			perror("output file open");
			exit(2);
		}
		int data;
		while(feof(fpi)==0) {
			data = GetByte(fpi, fPhase);
			if(data!=-1) fputc(data, fpo);
		}
	} else {
		// Data Dump
		char outfile[20];
		int type;				// 受信したブロックの種類  00:ヘッダ  01:データ 0xff:エンド
		int filetype;			// ファイルの種類  00:BASIC 01:ASCII 02:BINARY
		int total_len;
		int block_count;
		int size, start, exec;	// マシン語ファイルのときのスタートアドレス、サイズ、実行開始アドレス
		int state=0;			// 00:Waiting for header   01:Data receiving
		
		int remain=65536;		// マシン語のときのみ使用。最後の3バイトに開始アドレスがあり、それを取り出す場所を判定するのに使用する
		while(1) {
			int len = GetBlock(fpi, fPhase, buff, type);		// 1ブロック読み出し
			if(len==-1) break;
			if(len==-3) {
				puts("Check sum error");
				continue;
			}
			int i;
			switch(state) {
			case 0:						// ヘッダー待ち
				if(type!=0) break;		// ヘッダではなかったので無視
				for(i=0; i<8; i++) {
					outfile[i] = buff[i];
				}
				outfile[i] = '\0';
				// ファイル名の後ろのスペースを削除する
//				while(i>0) {
//					i--;
//					if(outfile[i]==' ') outfile[i]='\0'; else break;
//				}
				printf("\nFOUND:\"%s\"  ", outfile);
				// ファイルタイプの処理（出力ファイルの拡張子付けとか）
				filetype = buff[8];
				switch(filetype) {
				case 0:
					if(fVerbose) fputs("BASIC program", stdout);
					if(buff[9]==0xff && buff[10]==0xff) {
						if(fVerbose) fputs("(ASCII)", stdout);
						strcat(outfile, ".BAS");
					} else {
						if(fVerbose) fputs("(I-code)", stdout);
						strcat(outfile, ".IBA");
					}
					break;
				case 1:
					if(fVerbose) fputs("BASIC data file", stdout);
					strcat(outfile, ".TXT");
					break;
				case 2:
					if(fVerbose) fputs("Machine language program", stdout);
					strcat(outfile, ".BIN");
					break;
				default:
					printf("Unknown file type (%d)\n", buff[8]);
					break;
				}
				if(fVerbose) printf("\nOutput filename:\"%s\"\n", outfile); 
				total_len=0;
				fpo = fopen(outfile, "wb");
				block_count = 0;
				state = 1;				// データ受信状態に遷移
				break;
			case 1:
				// データ受信処理
				if(type==0xff) {		// ENDブロック受信時の処理
					if(fVerbose) {
						printf("LENGTH=$%04x", total_len);
						if(filetype==2) {
							printf(" START=$%04X END=$%04X EXEC=$%04x\n",
								(unsigned short)start, (unsigned short)(start+size-1), (unsigned short)exec);
						} else {
							fputs("\n", stdout);
						}
					}
					fclose(fpo);
					state = 0;			// ヘッダ待ちに遷移
					break;
				}
				if(type!=1) break;		// データブロックでないので無視
				switch(filetype) {		// ファイルタイプ別に受信処理を分ける
				case 0:		// BASIC Program
					total_len += len;
					for(i=0; i<len; i++) {
						fputc(buff[i], fpo);
					}
					break;
				case 1:		// BASIC data
					total_len += len;
					for(i=0; i<len; i++) {
						fputc(buff[i], fpo);
					}
					break;
				case 2:		// Machine language
					int start_pos = 0;
					if(block_count==0) {		// 最初のデータブロックの時はヘッダ情報を取り出す
						size = (buff[1]<<8) | buff[2];
						start = (buff[3]<<8) | buff[4];
						remain = size;		// 全体の残り転送数を設定
						start_pos = 5;		// ヘッダの分ダンプ開始位置をずらす
					}
					block_count++;
					for(int i=start_pos; i<len; i++) {
						if(remain>0) {
							fputc(buff[i], fpo);
							total_len++;
						} else {			// 残りサイズが0の場合は、最後に埋めてある実行開始アドレスを取得して終了
							exec = (buff[i+3]<<8) | buff[i+4];	// FF 00 00を読み飛ばす
							break;
						}
						remain--;
					}
					break;
				}
				break;
			}
		}
	
	}
	fclose(fpi);
}