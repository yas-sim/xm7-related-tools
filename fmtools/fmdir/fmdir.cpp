#include <stdio.h>
#include <stdlib.h>
#include "cfdimg.h"
#include "cfilesys.h"

// v0.0	初回リリース
// v0.1 -aオプション追加。クラスタチェインがループしてても抜け出せるよう制限値を設定
//      カタカナや漢字のファイル名が表示できなかったのを修正

#define VERSION "v0.1"

void usage( void ) {
	puts("Directry entry display tool for D77 format image " VERSION);
	puts("Programmed by an XM7 supporter\n");
	puts("Synopsis: FMDIR fdimagefile [options]");
	puts("Option: -c  Display cluster chain");
}

void main( int argc, char *argv[])
{
	if(argc<2) {
		usage();
		exit(1);
	}

	bool fChain = false;			// クラスタチェインを表示するかどうか
	bool fWide = false;				// 横表示をするか
	bool fAll = false;
	char imgname[512];
	// オプション解析
	imgname[0]='\0';
	for(int i=1; i<argc; i++) {
		switch(argv[i][0]) {
		case '/':
		case '-':
			switch(argv[i][1]) {
			case 'c':
			case 'C':
				fChain=true;
				break;
			case 'w':
			case 'W':
				fWide = true;
				break;
			case 'a':
			case 'A':
				fAll = true;
				break;
			default:
				break;
			}
			break;
		default:
			// オプション指定子が無い場合、ファイル名とみなす
			strncpy(imgname, argv[i], 510);
			break;
		}
	}

	if(strlen(imgname)==0) {
		usage();
		exit(1);
	}

	CFilesys fs;
	FDHANDLE *hFD = fs.FMMountFD(argv[1]);
	if(hFD==NULL) {
		puts("Failed to mount FD image file");
		exit(1);
	}
	CEntry ent;
	fs.FMGetFirstEntry(hFD, &ent);
	for(int i=0; i<151; i++) {
		if((unsigned char)ent.pFilename[0]!=0xff || fAll==true) {
			printf("%8s %1d %c %c", ent.pFilename, ent.nFileType,
				ent.fAscii==0?'B':'A', ent.fRandomAccess==0?'S':'R');
			int cluster = ent.nTopCluster, n = 1;
			// クラスタチェインを追いかける
			while(fAll==false) {		// fAll==trueの時にクラスタチェインを追いかけると無限ループに入ってしまう
				if(fChain) {
					// クラスタチェインを表示
					printf(n==1?" %d":"-%d", cluster);
				}
				cluster = fs.GetFATValue(hFD, cluster);
				if(cluster>=0xc0) break;		// 次のクラスタ番号が0xc0以上なら、チェインの終わり
				n++;
				if(n>255) break;		// クラスタチェインが異常に長いのでループ打ち切り
			}
			if(fChain) {
				// 最終クラスタ内の使用セクタ数を表示
				printf("<%d>", cluster-0xbf);
			}
			printf(fWide?" %d    ":" %d\n", n);
		}
		fs.FMGetNextEntry(hFD, &ent);
	}
	fs.FMUnmountFD(hFD);
}
