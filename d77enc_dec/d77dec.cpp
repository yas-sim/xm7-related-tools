#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION		"0.1(alpha)"

bool g_fVerbose;

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
	puts("d77dec infile outfile");
	puts("infile : .d77 file name");
	puts("outfile: .txt file name");
}


void SectorDecode(FILE *fpi, FILE *fpo, void *img )
{
	SECTORIMAGE *simg = (SECTORIMAGE*)img;
	fputs(       "#   CC HH RR NN DN DMK ST SIZE\n", fpo);
	fprintf(fpo, "ID= %02X %02X %02X %02X %s %s %02X ",
				(unsigned char)simg->C, (unsigned char)simg->H,
				(unsigned char)simg->R, (unsigned char)simg->N,
				simg->Density==0?"DD":"SD",
				simg->DDM==0?" DM":"DDM",
				(unsigned char)simg->Status);
	fprintf(fpo, "%d\n", simg->SectorSize);
	for(int i=0; i<simg->SectorSize; i++) {
		fprintf(fpo, "%02X", (unsigned char)simg->SectorData[i]);
		if((i & 0x0f)==0x0f) fprintf(fpo, "\n"); else fprintf(fpo, " ");
	}
}

void TrackDecode(FILE *fpi, FILE *fpo, void *img )
{
	DISKIMAGE *dimg = (DISKIMAGE*)img;
	for(int trk=0; trk<164; trk++) {
		if(g_fVerbose) printf("Track %d", trk);
		fprintf(fpo, "Track= %d\n", trk);
		if(dimg->TrackOffset[trk]==0) { if(g_fVerbose) printf("..SKIP\n"); continue; }
		//SECTORIMAGE *simg = (SECTORIMAGE*)((int)dimg+(dimg->TrackOffset[trk]));
		SECTORIMAGE *simg = (SECTORIMAGE*)((char*)dimg+(dimg->TrackOffset[trk]));
		int SPT = simg->SPT;
		for(int sct=0; sct<SPT; sct++) {
			if(g_fVerbose) printf(".");
			SectorDecode(fpi, fpo, (void*)simg);
			simg=(SECTORIMAGE*)((char*)simg+simg->SectorSize+16);
		}
		if(g_fVerbose) printf("\n");
	}
}

void ImageExtract( char *imagename, char *extractname )
{
	FILE *fpi;
	fpi = fopen(imagename, "rb");
	if(fpi==NULL) {
		perror("Input File Open");
		exit(-1);
	}

	FILE *fpot;
	fpot = fopen(extractname, "wt");
	if(fpot==NULL) {
		perror("Output File Open");
		exit(-1);
	}


	fseek(fpi, 0, SEEK_END);
	long nFileSize = ftell(fpi);
	if(g_fVerbose) printf("File Size=%d\n", nFileSize);
	fseek(fpi, 0, SEEK_SET);

	char *lpImageBuffer = new char[nFileSize];
	fread((void*)lpImageBuffer, 1, nFileSize, fpi);
	DISKIMAGE *dimg;
	dimg = (DISKIMAGE*)lpImageBuffer;
	fprintf(fpot, "DiskName____= %s\n", dimg->DiskName);
	fprintf(fpot, "WriteProtect= %s\n", dimg->WriteProtect==0?"OFF":"ON");
	fprintf(fpot, "MediaType___= %s\n", dimg->MediaType==0?"2D":dimg->MediaType==0x10?"2DD":"2HD");
	fprintf(fpot, "DiskSize____= %d\n", dimg->DiskSize);

	TrackDecode(fpi, fpot, (void*)lpImageBuffer);

	delete []lpImageBuffer;
	fclose(fpi);
}

int main( int argc, char *argv[])
{
	g_fVerbose = false;
	char infile[512], outfile[512];
	infile[0] = '\0';
	outfile[0] = '\0';
	for(int i=1; i<argc; i++) {
		switch(argv[i][0]) {
		case '-':
		case '/':
			switch(argv[i][1]) {
			case 'v':
				g_fVerbose = true;
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
	if(g_fVerbose) {
		puts("[D77DEC]D77 file to text file decoder " VERSION);
		puts("Programmed by an XM7 supporter");
	}
	if(strlen(infile)==0 || strlen(outfile)==0) {
		Usage();
		exit(-1);
	}
	ImageExtract(infile, outfile);
	
	return 0;
}

