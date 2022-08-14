#include "CFILE.H"
#include "CWAVE.H"
#include "CPreCalc.h"
#include "CLPF.h"
#include "CAGC.h"
#include "CComparator.h"

#include <stdio.h>
#include <stdlib.h>

#define CENTER	(0x8000)
#define VERSION	"v0.4"

// v0.0 20000229 初期公開バージョン
// v0.1 20000229 -th, -tlオプション追加
// v0.2 20000301 録音レベルの検出方法に標準偏差を使うように変更
// v0.3 20000303 LPFの方式を積分方式に変更
// v0.3 20000521 位相を指定できるようにした
// v0.4 20000611 AGCオプションを追加,自動レベル解析を廃止

void usage( void )
{
	puts("WAV file to T77 file converter for XM7 " VERSION "\n");
	puts("Synopsis: wav2t77 infile outfile [options]\n");
	puts("Option : -v       Verbose mode");
	puts("         -th[hex] Specify upper threshold in hex code");
	puts("         -tl[hex] Specify lower threshold in hex code\n");
	puts("         -p[mode] Specify phase");
	puts("                  mode: r=Reverse, n=Normal[default]");
	puts("         -agc     AGC on");
	puts("         -h       Show this message");
	puts("ex1. wav2t77 test.wav test.t77");
	puts("ex2. wav2t77 test.wav test.t77 -v -th8400 -tl7d00");
}

unsigned long htol( char *str ){
	unsigned long result = 0ul;
	while(*str) {
		result = result*0x10 + ((*str<='9')?(*str-'0'):((*str & ~0x20)-'A'+10));
		str++;
	}
	return result;
}

void main( int argc, char *argv[] ) {
	CWaveFile wav;
	FILE *fo;

	if(argc<3) {
		usage();
		exit(0);
	}

	// オプション解析
	int source=0, dest=0;
	bool fVerbose = false;
	bool fPhase=true;		// 音声ファイルの位相(true=Normal False=Reverse)
	bool fAGC = false;
	unsigned long pth=0xffff, ptl=0;
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
			case 't':
			case 'T':
				// Thresholdレベル指定
				switch(argv[i][2]) {
				case 'h':
				case 'H':
					// Thの指定
					pth = htol(argv[i]+3);
					if(pth<ptl) {
						puts("Illegal parameter. th must be higher than tl.");
						exit(1);
					}
					break;
				case 'l':
				case 'L':
					// Tlの指定
					ptl = htol(argv[i]+3);
					if(ptl>pth) {
						puts("Illegal parameter. tl must be lower than th.");
						exit(1);
					}
					break;
				}
				break;
			case 'a':
			case 'A':			// AGCオプション
				if(stricmp(argv[i]+1, "AGC")==0) {
					fAGC = true;
				}
				break;
			default:
				break;
			}
		} else {
			// オプション指定子がない場合、ファイル名とみなす
			if(source==0)	source=i;
			else			dest=i;
		}
	}

	if(fVerbose) {
		puts("WAV to T77 converter " VERSION);
		puts("Programmed by an XM7 supporter");
	}
	
	unsigned long NoiseLevel, High, Low;
	if(pth==0xffff || ptl==0) {
//		// オプションでスレショルドが指定されていない場合、
//		// 入力WAVファイルを1パス通して、パラメータを計算
//		CPreCalc pc;
//		if(fVerbose) {
//			puts("\nPASS1: Analyzing record level..."); 
//		}
//		pc.CalcParam(argv[1], &NoiseLevel, &High, &Low, fPhase);
		// 自動レベル解析は無意味なので廃止
		NoiseLevel = 0x8000;
		High = 0x8200;
		Low = 0x7e00;
	} else {
		// オプションでスレショルドが指定されている場合
		NoiseLevel = 0x8000;
		High = pth;
		Low = ptl;
	}
	if(fVerbose) {
		printf("Comparator threshold  High 0x%04x   Low 0x%04x\n", High, Low);
	}
	CComparator cmp;
	cmp.SetThreshold((High-CENTER)/2+CENTER, 
					 CENTER-(CENTER- Low)/2);		// コンパレータのスレショルドを設定

	if(wav.Open(argv[1], FMODE_READ | FMODE_BIN)==false) {
		perror("Source file open");
		exit(1);
	}
	if((fo=fopen(argv[2], "wb"))==NULL) {
		perror("Destination file open");
		exit(2);
	}

	if(fVerbose) {
		printf("Sample freq %d[Hz]   Channel %d[ch]  Quantize bit %d[bit]  Phase %s\n", wav.GetSplRate(), wav.GetNOChanell(), 
			wav.GetBitPerSpl(), fPhase==true?"Normal":"Reverse");
		if(fAGC) puts("AGC on"); else puts("AGC off");
//		puts("PASS2: Converting...");
	}
	double period = 1.0/wav.GetSplRate();		// 1サンプル周期を求める
	fputs("XM7 TAPE IMAGE 0", fo);
	// マーカー出力
	fputc(0x00, fo);
	fputc(0x00, fo);

	double Length=0.0;
	bool data=false, last=false;
	unsigned short snd;
	CLPF lpf;
	CAGC agc;
	wav.Seek(0x3a);		// wavファイルのヘッダを飛ばす
	while(!wav.eof()) {
		snd = wav.ReadWavDataMono();

		if(fPhase==false) snd = 0xffff-snd;		// 位相反転

		// LPFに通す
		lpf.DataInput(snd);
		snd = (unsigned short)lpf.DataOutput();

		// AGCをかける
		if(fAGC) {
			agc.DataInput(snd);
			snd = (unsigned short)agc.DataOutput();
		}

		data = cmp.Comparate(snd);				// 音声データをコンパレータに入力
		if(data==last) {
			// 前回と比較結果が同じなので継続期間を増やす
			Length += period;
		} else {
			// true<->falseが切り替わったので、継続期間のデータを出力
			long p = (long)((Length+period)/9e-6);
			Length = 0.0;
			while(p>0) {
				long p1;
				if(p>0x7fff) p1 = 0x7fff;
				else		 p1 = p;
				if(data==false) {
					// 0の期間のデータ出力
					fputc(((p1>>8) | 0x80) & 0x00ff, fo);
					fputc(  p1             & 0x00ff, fo);
				} else {
					// 1の期間のデータ出力
					fputc((p1>>8) & 0x007f, fo);
					fputc( p1     & 0x00ff, fo);
				}
				p -= p1;
			}
		}
		last = data;

	}
	fclose(fo);
	wav.Close();
	if(fVerbose)	puts("Done.");
}