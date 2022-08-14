#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION		"0.1(alpha)"

typedef struct {
	char	DiskName[17];
	char	Reserve[9];
	char	WriteProtect;	// 00:Not	10:WriteProtect
	char	MediaType;		// 00:2D	10:2DD	20:2HD
	long	DiskSize;
	long	TrackOffset[164];
} DISKIMAGE;

typedef struct {
	char	C;
	char	H;
	char	R;
	char	N;
	short	SPT;		// Sector per track
	char	Density;	// 00:倍密 40:単密
	char	DDM;		// Deleted data mark 00:Normal 10:DDM
	char	Status;
	char	Reserve[5];
	short	SectorSize;
	char	SectorData[1];
} SECTORIMAGE;


void Usage( void )
{
	puts("d77enc infile outfile");
	puts("infile : .txt file name");
	puts("outfile: .d77 file name");
}


void main(int argc, char *argv[])
{
	char infile[512], outfile[512];
	infile[0] = '\0';
	outfile[0] = '\0';
	bool fVerbose = false;
	for(int i=1; i<argc; i++) {
		switch(argv[i][0]) {
		case '-':
		case '/':
			switch(argv[i][1]) {
			case 'v':
			case 'V':
				fVerbose = true;
				break;
			case 'h':
			case 'H':
			case '?':
				Usage();
				exit(0);
				break;
			default:
				break;
			}
			break;
		default:
			if(strlen(infile)==0) {
				strcpy(infile, argv[i]);	// infileが空ならinfileにargvを設定
			} else {
				strcpy(outfile, argv[i]);	// infileになにか入っていればoutfileにargvを設定
			}
			break;
		}
	}
	if(fVerbose) {
		puts("[D77ENC]Text file to D77 file encoder " VERSION);
		puts("Programmed by an XM7 supporter");
	}

	if(strlen(infile)==0 || strlen(outfile)==0) {
		Usage();
		exit(-1);
	}

	FILE *fpi;
	if((fpi=fopen(infile, "rt"))==NULL) {
		perror("File open");
		exit(-1);
	}
	DISKIMAGE *dimg = (DISKIMAGE*)new char[2*1024*1024];	// 2MB用意する
	char *wptr = (char*)dimg + (17+9+1+1+4+4*164);			// セクタ情報書き込みポインタ(DISKIMAGEの構造体分進めておく)
	// DISKIMAGE構造体に適当な初期値を代入しておく
	strcpy(dimg->DiskName, "Untitled");
	dimg->DiskSize = 0;
	dimg->MediaType = 0;		// 2D
	dimg->WriteProtect = 0;		// No protect
	for(int i=0; i<164; i++) dimg->TrackOffset[i]=0;		// トラックオフセット初期化

	int CurrentTrack = -1;		// 現在処理中のトラック番号
	int NumberOfSector = 0;		// セクター数
	SECTORIMAGE *SectorTable[512];	// セクタ位置を記録するテーブル(トラックの切り替わり時にSPTを書き換えるために使用)
	char cmd[256], ope[256];
	while(!feof(fpi)) {
		fscanf(fpi, "%s", cmd);
		fgetc(fpi);					// スペース１文字読み飛ばし
		fgets(ope, 255, fpi);
		if(cmd[0]=='#' || cmd[0]==';') continue;					// '#', ';'からコメントとみなす
		for(char *ptr=ope; *ptr; ptr++) if(*ptr<' ') *ptr='\0';		// 文字列中の制御コードを'\0'に置き換え
		if(stricmp("DiskName____=", cmd)==0) {
			strncpy(dimg->DiskName, ope, 16);
			dimg->DiskName[16] = '\0';
		}
		if(stricmp("WriteProtect=", cmd)==0) {
			if(stricmp(ope, "ON")==0) dimg->WriteProtect = 0x10;
			else if(stricmp(ope, "OFF")==0) dimg->WriteProtect = 0;
		}
		if(stricmp("MediaType___=", cmd)==0) {
			if(stricmp(ope, "2D")==0) dimg->MediaType = 0;
			else if(stricmp(ope, "2DD")==0) dimg->MediaType = 0x10;
			else if(stricmp(ope, "2HD")==0) dimg->MediaType = 0x20;
		}
		if(stricmp("ID=", cmd)==0) {
			if(NumberOfSector<512) {
				if(fVerbose) printf(".");
				char ddm[4], density[4];
				SECTORIMAGE *sct = SectorTable[NumberOfSector++] = (SECTORIMAGE*)wptr;
				wptr+=(1+1+1+1+2+1+1+1+5+2);		// SECTORIMAGE構造体分進めておく
				sct->Density	= 0x00;		// 倍密
				sct->DDM		= 0x00;		// Normal Mark
				// 文字列解析
				int c,h,r,n,status,size;
				sscanf(ope, "%02X %02X %02X %02X %s %s %02X %d",
					&c, &h, &r, &n, density, ddm, &status, &size);
					sct->C = c;
					sct->H = h;
					sct->R = r;
					sct->N = n;
					sct->Status = status;
					sct->SectorSize = size;
				if(stricmp(density, "SD" )==0) sct->Density	= 0x40;	// 単密
				if(stricmp(ddm    , "DDM")==0) sct->DDM		= 0x10;	// DDM
				// データ部読み取り
				for(int i=0; i<sct->SectorSize; i++) {
					fscanf(fpi, "%02X", &n);
					*wptr++ = n;
				}
			}
		}
		// トラックタグ処理
		if(stricmp("TRACK=", cmd)==0) {
			int tmp = atoi(ope);
			if(tmp>CurrentTrack && tmp<164) {
				if(fVerbose) printf("\nTrack %3d", tmp);
				// 現在のトラック番号より大きい場合のみトラック番号を設定する
				// すべてのセクタにSPT値を設定する
				for(int i=0; i<NumberOfSector; i++) {
					SectorTable[i]->SPT = NumberOfSector;
				}
				dimg->TrackOffset[tmp] = (long)((long)wptr-(long)dimg);
				CurrentTrack = tmp;
				NumberOfSector = 0;
			}
		}
	}
	dimg->DiskSize = (long)wptr-(long)dimg;
	FILE *fpo = fopen(outfile, "wb");
	fwrite((void*)dimg, 1, dimg->DiskSize, fpo);
	fcloseall();
	delete []dimg;
}
