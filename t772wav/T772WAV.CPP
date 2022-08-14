#include "CFILE.H"
#include "CWAVE.H"

#include <stdio.h>
#include <stdlib.h>

#define VERSION	"v0.1a"

void usage( void )
{
	puts("T77 file to WAV file converter for XM7 " VERSION "\n");
	puts("Synopsis: t772wav infile outfile [options]\n");
	puts("Option : -v       Verbose mode");
	puts("         -h       Show this message");
	puts("ex. t772wav test.t77 test.wav");
}

void main(int argc, char *argv[])
{
	CWaveFile wav;
	FILE *fpi;

	if(argc<3) {
		usage();
		exit(0);
	}

	// オプション解析
	int source=0, dest=0;
	bool fVerbose = false;
	for(int i=1; i<argc; i++) {
		if(argv[i][0]=='-' || argv[i][0]=='/') {
			switch(argv[i][1]) {
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
		puts("T77 to WAV converter " VERSION);
		puts("Programmed by an XM7 supporter");
	}
	if(source==0 || dest==0) {
		puts("Specify source and destination file name properly\n");
		exit(3);
	}

	if((fpi=fopen(argv[source], "rb"))==NULL) {
		perror("Source file open");
		exit(1);
	}

	if(wav.Open(argv[dest], FMODE_WRITE | FMODE_BIN)==false) {
		perror("Destination file open");
		exit(2);
	}

	char buff[1024];
	// ヘッダーチェック!
	fgets(buff, 20, fpi);
	if(strcmp(buff, "XM7 TAPE IMAGE 0")!=0) {
		puts("Source file is not a valid XM7 tape image file");
		exit(1);
	}
	fseek(fpi, 0x12, SEEK_SET);		// ヘッダ読み飛ばし
	
	double len=0.0;	// 長さ
	int pol=0;		// 極性
	while(1) {
		int ch1 = fgetc(fpi);
		int ch2 = fgetc(fpi);
		if(feof(fpi)) break;
		int tmp_len = (ch1<<8) | ch2;					// 長さ生成
		int tmp_pol = ((tmp_len & 0x8000)==0?0:1);		// 極性判定
		tmp_len &= 0x7fff;
		tmp_len *= 9;									// 9掛けて時間に直す(μs)
		if(pol!=tmp_pol) {
			const double wav_period = 1.0e6/44.1e3;			// wavファイルの周期(μs)
			while(len>0) {
				int sound=0x80-(pol==0?-1:1)*0x40;
				wav.WriteSnd(sound);
				len -= wav_period;
			}
			pol = tmp_pol;
		}
		len+=tmp_len;
	}
	wav.Close();
}
		


