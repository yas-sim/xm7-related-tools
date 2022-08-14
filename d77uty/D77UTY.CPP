#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION	"0.1a"

// d77tool test.d77 ADD test2.d77 2
// d77tool test.d77 ADD test2.d77
// d77tool test.d77
// d77tool test.d77 L
// d77tool test.d77 EXT test2.d77 3
// d77tool test.d77 P+ 3
// d77tool test.d77 P- ALL
// d77tool test.d77 N name
// d77tool test.d77 ORD 1423
// d77tool test.d77 DEL 2

typedef struct {
	char	DiskName[17];
	char	Reserve[9];
	char	WriteProtect;	// 00:Not	10:WriteProtect
	char	MediaType;		// 00:2D	10:2DD	20:2HD
	long	DiskSize;
	long	TrackOffset[164];
} DISKIMAGE;

// 指定の番号のディスクイメージへのオフセットを返す
// ディスク番号は先頭のディスクが1
// 指定の番号よりディスク数が少なかった場合-1を返す
long GetDiskOffset( FILE *fp, int n ) {
	long original_pos = ftell(fp);
	fseek(fp, -1, SEEK_END);
	long fileend = ftell(fp);		// ディスクイメージ全体の大きさを取得
	DISKIMAGE img;
	long pos = 0L;
	for(int i=1; i<n; i++) {
		fseek(fp, pos, SEEK_SET);
		fread((void*)&img, 1, 0x2b0, fp);
		pos += img.DiskSize;		
		if(pos>=fileend) {
			pos = -1;
			break;
		}
	}
	fseek(fp, original_pos, SEEK_SET);
	return pos;
}

bool ReadHeader( FILE *fp, int pos, DISKIMAGE *img ) {
	long ofst = GetDiskOffset(fp, pos);
	if(ofst==-1) return false;
	long original_pos = ftell(fp);
	fseek(fp, ofst, SEEK_SET);
	fread((void*)img, 1, 0x2b0, fp);
	fseek(fp, original_pos, SEEK_SET);
	return true;
}

bool WriteHeader( FILE *fp, int pos, DISKIMAGE *img ) {
	long ofst = GetDiskOffset(fp, pos);
	if(ofst==-1) return false;
	long original_pos = ftell(fp);
	fseek(fp, ofst, SEEK_SET);
	fwrite((void*)img, 1, 0x2b0, fp);
	fseek(fp, original_pos, SEEK_SET);
	return true;
}

int GetNumberOfDisks( FILE *fp ) {
	DISKIMAGE dimg;
	int i=1;
	while(1) {
		if(ReadHeader(fp, i, &dimg)==false) return i-1;
		i++;
	}
	return -1;
}

void ListUp( FILE *fp ) {
	DISKIMAGE dimg;
	int nod = GetNumberOfDisks(fp);
	for(int i=1; i<=nod; i++) {
		ReadHeader(fp, i, &dimg);
		dimg.DiskName[16]='\0';
		printf("%2d : '%-16s' WP=%s\n", i, dimg.DiskName, dimg.WriteProtect==0?"OFF":"ON");
	}
}

void Add( char *filename1, FILE *fp, char *filename2, int order ) {
	FILE *fpi = fopen(filename2, "rb");
	if(fpi==NULL) {
		puts("Attempted to open add file, but failed");
		return;
	}
	FILE *fptmp = fopen("D77UTY.TMP", "w+b");
	if(fptmp==NULL) {
		puts("Failed to create temp file");
		return;
	}
	int nod = GetNumberOfDisks(fp);
	DISKIMAGE dimg;
	for(int disk=1; disk<=nod; disk++) {
		ReadHeader(fp, disk, &dimg);
		fseek(fp, GetDiskOffset(fp, disk), SEEK_SET);
		if(disk==order) {			// ディスク番号とorderが一致したら、追加するファイルを書き込む
			while(1) {
				int ch = fgetc(fpi);
				if(ch<0 || feof(fpi)) break;
				fputc(ch, fptmp);
			}
		}
		for(int j=0; j<dimg.DiskSize; j++) {
			fputc(fgetc(fp), fptmp);
		}
	}
	if(order==0) {					// orderが0の場合は最後に追加する
		while(1) {
			int ch = fgetc(fpi);
			if(ch<0 || feof(fpi)) break;
			fputc(ch, fptmp);
		}
	}
	fcloseall();
	// d77uty.tmpのファイル名を元のファイル名に変更する
	remove(filename1);
	rename("D77UTY.TMP", filename1);
}

// 指定のディスク番号のイメージを抜き取る
void Extract( char *filename1, FILE *fp, char *filename2, int order ) {
	FILE *fpo = fopen(filename2, "wb");
	if(fpo==NULL) {
		puts("Attempted to open extract file, but failed");
		return;
	}
	FILE *fptmp = fopen("D77UTY.TMP", "wb");
	if(fptmp==NULL) {
		puts("Failed to create temp file");
		return;
	}
	int nod = GetNumberOfDisks(fp);
	DISKIMAGE dimg;
	for(int disk=1; disk<=nod; disk++) {
		ReadHeader(fp, disk, &dimg);
		fseek(fp, GetDiskOffset(fp, disk), SEEK_SET);
		if(disk==order) {			// ディスク番号とorderが一致したら、削除するファイルを書き出す
			for(int j=0; j<dimg.DiskSize; j++) {
				fputc(fgetc(fp), fpo);
			}
		} else {					// 違う場合はテンポラリファイルに書き出す
			for(int j=0; j<dimg.DiskSize; j++) {
				fputc(fgetc(fp), fptmp);
			}
		}
	}
	fcloseall();
	// d77uty.tmpのファイル名を元のファイル名に変更する
	remove(filename1);
	rename("D77UTY.TMP", filename1);
}

// 指定のディスク番号のイメージをコピーする(元のファイルに変化ナシ)
void Copy( FILE *fp, char *filename2, int pos ) {
	FILE *fpo = fopen(filename2, "wb");
	if(fpo==NULL) {
		puts("Attempted to open copy file, but failed");
		return;
	}
	int nod = GetNumberOfDisks(fp);
	DISKIMAGE dimg;
	for(int disk=1; disk<=nod; disk++) {
		ReadHeader(fp, disk, &dimg);
		fseek(fp, GetDiskOffset(fp, disk), SEEK_SET);
		if(disk==pos) {			// ディスク番号とposが一致したら、コピーするファイルに書き出す
			for(int j=0; j<dimg.DiskSize; j++) {
				fputc(fgetc(fp), fpo);
			}
		}
	}
	fcloseall();
}

// プロテクトフラグのON/OFFを変更する
void ProtectFlag( FILE *fp, bool flag, char *pos ) {
	int nod = GetNumberOfDisks(fp);
	int p;
	if(stricmp(pos, "ALL")==0) {
		p = -1;
	} else {
		p = atoi(pos);
		if(p<1 || p>nod) return;
	}
	DISKIMAGE dimg;
	for(int i=1; i<=nod; i++) {
		if(p!=-1 && i!=p) continue;		// ALL指定のとき以外は、ディスク番号と一致しない場合は処理を飛ばす
		ReadHeader(fp, i, &dimg);
		dimg.WriteProtect = (flag==false?0:0x10);
		WriteHeader(fp, i, &dimg);
	}
}

// ディスクイメージの名前を変更する
void NameChange( FILE *fp, char *name, int order ) {
	int nod = GetNumberOfDisks(fp);
	if(order<1 || order>nod) return;
	DISKIMAGE dimg;
	ReadHeader(fp, order, &dimg);
	strncpy(dimg.DiskName, name, 16);
	dimg.DiskName[16]='\0';
	WriteHeader(fp, order, &dimg);
}

// 指定の順番にディスクイメージを並べ替える
void OrderChange( char *filename1, FILE *fp, char *order ) {
	FILE *fptmp = fopen("D77UTY.TMP", "wb");
	if(fptmp==NULL) {
		puts("Failed to create temp file");
		return;
	}
	int nod = GetNumberOfDisks(fp);
	DISKIMAGE dimg;
	for(int i=0; i<(int)strlen(order); i++) {
		int ord = order[i] & 0x0f;		// 番号の文字を数値に変換
		if(ord<1 || ord>nod) continue;	// おかしな数値の場合無視
		ReadHeader(fp, ord, &dimg);
		fseek(fp, GetDiskOffset(fp, ord), SEEK_SET);
		for(int j=0; j<dimg.DiskSize; j++) {
			fputc(fgetc(fp), fptmp);
		}
	}
	fcloseall();
	// d77uty.tmpのファイル名を元のファイル名に変更する
	remove(filename1);
	rename("D77UTY.TMP", filename1);
}

// 指定の番号のイメージを消去する
void Delete( char *filename1, FILE *fp, char *pos ) {
	FILE *fptmp = fopen("D77UTY.TMP", "wb");
	if(fptmp==NULL) {
		puts("Failed to create temp file");
		return;
	}
	int nod = GetNumberOfDisks(fp);
	int p = atoi(pos);
	DISKIMAGE dimg;
	for(int i=1; i<=nod; i++) {
		if(p==i) continue;				// 指定のディスク番号のときはスキップする
		ReadHeader(fp, i, &dimg);		// それ以外の場合はコピーする
		fseek(fp, GetDiskOffset(fp, i), SEEK_SET);
		for(int j=0; j<dimg.DiskSize; j++) {
			fputc(fgetc(fp), fptmp);
		}
	}
	fcloseall();
	// d77uty.tmpのファイル名を元のファイル名に変更する
	remove(filename1);
	rename("D77UTY.TMP", filename1);
}

void Usage( void ) {
	puts("D77UTY INFILE [ADD|+] ADDFILE IMG#    ディスクイメージの追加");
	puts("D77UTY INFILE {L}                     イメージリスト表示");
	puts("D77UTY INFILE [EXT|-] EXTFILE IMG#    イメージ切り出し");
	puts("D77UTY INFILE CPY     CPYFILE IMG#    イメージコピー");
	puts("D77UTY INFILE P[+|-]  [IMG#|ALL]      ライトプロテクト設定");
	puts("D77UTY INFILE NAM     IMGNAME IMG#    イメージ名変更");
	puts("D77UTY INFILE ORD     ORDER           イメージ順序変更");
	puts("INFILE, ADDFILE, EXTFILE, CPYFILE : '.D77'形式ファイル");
	puts("IMG#    : イメージ番号(1～)");
	puts("IMGNAME : イメージ名(１６文字まで）");
	puts("ORDER   : ANK数字１文字で順序を指定");
	puts("extとcpyの違い: extの場合切り出したイメージはINFILE中から削除される。cpyは削除されない");
	puts("e.g.");
	puts("d77uty test.d77 add add.d77 2  2番目にadd.d77を追加");
	puts("d77uty test.d77                含まれているイメージ一覧表示");
	puts("d77uty test.d77 ext ext.d77 3  3番目のイメージをext.d77に出力");
	puts("d77uty test.d77 cpy copy.d77 2 2番目のイメージをcopy.d77にコピー");
	puts("d77uty test.d77 p- all         全てのイメージのライトプロテクト解除");
	puts("d77uty test.d77 nam game 1     1番目のイメージ名を'game'に設定");
	puts("d77uty test.d77 ord 312        3, 1, 2の順にイメージを並べ替え");
}

void main( int argc, char *argv[] )
{
	puts("D77 image manipulation program " VERSION);
	puts("Programmed by an XM7 supporter");
	
	if(argc<2) {
		puts("Too few parameters\n");
		Usage();
		exit(-1);
	}
	FILE *fp;
	if((fp=fopen(argv[1], "r+b"))==NULL) {
		puts("Attempted to open input file, but failed.");
		return;
	}
	int order = 0;
	switch(argc) {
	case 2:
		ListUp(fp);
		break;
	case 3:
		if(stricmp(argv[2], "L")==0) {
			ListUp(fp);
		}
		break;
	case 5:
		order = atoi(argv[4]);
		if(stricmp(argv[2], "NAM")==0) {
			NameChange(fp, argv[3], order);
			break;
		} else if(stricmp(argv[2], "EXT")==0 || stricmp(argv[2], "-")==0) {
			Extract(argv[1], fp, argv[3], order);
			break;
		} else if(stricmp(argv[2], "CPY")==0) {
			Copy(fp, argv[3], order);
			break;
		}
		// break;
	case 4:
		if(stricmp(argv[2], "ADD")==0 || stricmp(argv[2], "+")==0) {
			Add(argv[1], fp, argv[3], order);
		} else if(toupper(argv[2][0])=='P' && argc>2) {
			ProtectFlag(fp, argv[2][1]=='-'?false:true, argv[3]);
		} else if(stricmp(argv[2], "ORD")==0) {
			OrderChange(argv[1], fp, argv[3]);
		} else if(stricmp(argv[2], "DEL")==0) {
			Delete(argv[1], fp, argv[3]);
		}
		break;
	default:
		break;
	}
	fcloseall();
}