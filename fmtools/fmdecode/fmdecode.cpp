#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "CMotorolaS.h"

// v0.1 "iba"�g���q���󂯕t����悤�ɕύX

#define VERSION		"v0.1"

class CFMDECODE {
protected:
	HANDLE hInFile, hOutFile;	// ���o�̓t�@�C���ւ̃n���h��
	char infile[512], outfile[512];			// �o�̓t�@�C����

	bool fRaw;					// RAW���[�h�t���O

	char filename[9];
	unsigned char FileType;
	bool Ascii;
	bool Random;
	bool fVerbose;

protected:
	void DecodeBasicFile( void ) {
		unsigned char buff[4];
		DWORD NOW, NOR;
		BOOL status;
		status = ReadFile(hInFile, buff, 2, &NOR, NULL);		// Dummy read (skip 2 bytes)
		while(1) {
			status = ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR>0) {
				WriteFile(hOutFile, &buff, 1, &NOW, NULL);
//				printf("%c", buff[0]);
				if(Ascii==true) {
					if(buff[0]==0x1a) break;
				}
			} else break;
		}
	}

	void DecodeRawFile( void ) {
		unsigned char buff[4];
		DWORD NOW, NOR;
		BOOL status;
		while(1) {
			status = ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR>0) {
				WriteFile(hOutFile, &buff, 1, &NOW, NULL);
			} else break;
		}
	}

	void DecodeAsciiFile( void ) {
		unsigned char buff[4];
		DWORD NOW, NOR;
		BOOL status;
		while(1) {
			status = ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR>0) {
				WriteFile(hOutFile, &buff, 1, &NOW, NULL);
//				printf("%c", buff[0]);
				if(Ascii==true) {
					if(buff[0]==0x1a) break;
				}
			} else break;
		}
	}

	void DecodeMachineFile( void )
	{
		unsigned char buff[4];
		unsigned long nFileSize;
		unsigned short nStartAddr, nEntryAddr;
		DWORD NOW, NOR;
		BOOL status;
		status = ReadFile(hInFile, buff, 3, &NOR, NULL);
		nFileSize = (buff[0]<<16) | (buff[1]<<8) | buff[2];
//		printf("File size=%06x\n", nFileSize);
		status = ReadFile(hInFile, buff, 2, &NOR, NULL);
		nStartAddr = (buff[0]<<8) | buff[1];
		char *data = new char[nFileSize+1];
		// �f�[�^�{�̂�ǂݏo��
		status = ReadFile(hInFile, data, nFileSize, &NOR, NULL);
		if(status==0 && NOR==0) {
			puts("Unexpected EOF");
			delete []data;
			exit(1);
		}
		// Dummy��ǂݔ�΂�(���̂R�o�C�g�͉��̂��߂ɂ���񂾂낤?)
		status = ReadFile(hInFile, buff, 3, &NOR, NULL);
		if(status==0 && NOR==0) {
			puts("Unexpected EOF(1)");
			delete []data;
			exit(1);
		}
		// �G���g���[�A�h���X��ǂݏo��
		status = ReadFile(hInFile, buff, 2, &NOR, NULL);
		if(status==0 && NOR==0) {
			puts("Unexpected EOF(1)");
			delete []data;
			exit(1);
		}
		nEntryAddr = (buff[0]<<8) | buff[1];
		if(fVerbose) 
			printf("StartAddr=%04x EndAddr=%04x Entry=%04x\n", 
				nStartAddr, nStartAddr+nFileSize-1, nEntryAddr);

		// ���g���[��S�t�H�[�}�b�g�Ńf�[�^���o��
		CMotorolaS mot;
		char outbuff[512];
		// �f�[�^���R�[�h�𐶐�
		for(unsigned int adr=0; adr<nFileSize; adr+=0x20) {
			mot.InitSRecord(nStartAddr+adr);
			for(unsigned int ofst=0; ofst<0x20; ofst++) {
				if(adr+ofst>=nFileSize) break;
				mot.AddData(data[adr+ofst]);
			}
			mot.GetSRecordBuff(outbuff);
			strcat(outbuff, "\n");
			WriteFile(hOutFile, outbuff, strlen(outbuff), &NOW, NULL);
//			puts(outbuff);
		}
		// �G���h���R�[�h�𐶐�
		mot.InitSRecord(nEntryAddr);
		mot.SetRecordType(9);
		mot.GetSRecordBuff(outbuff);
		strcat(outbuff, "\n");
		WriteFile(hOutFile, outbuff, strlen(outbuff), &NOW, NULL);
//		puts(outbuff);
		delete []data;
	}

	bool ReadHeader( void ) {
		DWORD NOR;
		unsigned char header[0x10];
		ReadFile(hInFile, header, 0x10, &NOR, NULL);
		if(header[13]!='X' || header[14]!='M' || header[15]!='7') return false;
		// �t�@�C�����������
		for(int i=0; i<9; i++) {
			filename[i] = header[i];
		}
		FileType = header[10];
		Ascii  = (header[11]==0?false:true);
		Random = (header[12]==0?false:true);
		return true;
	}

	// �w�b�_����\������
	void ShowHeader( void ) {
		if(fVerbose) 
			printf("FileName=%-8s FileType=%d Ascii=%c Random=%c\n", filename, FileType, Ascii?'A':'B', Random?'R':'S');
	}

public:

	CFMDECODE() {
		fRaw = true;
		fVerbose=false;
	}

	void Usage( void )	{
		puts("FM-FILE to DOS/V file decoder " VERSION);
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMDECODE FM_FILE [options]");
		puts("Options: -v  Verbose mode");
		puts("e.g. FMDECODE test.0a0\n");
		puts("Note: The FM_FILE must be created by FMREAD command.");
		puts("Output filename will be genarated automatically");
		puts("  0 A 0 file -> .BAS");
		puts("  1 A 0 file -> .TXT");
		puts("  2 B 0 file -> .MOT");
		puts(" Other  file -> .IMG");
	}
	
	void Ignition( int argc, char*argv[]) {
		if(argc<2) {
			Usage();
			exit(1);
		}
		
		strncpy(infile , argv[1], 510);	// ���̓t�@�C��(FM�`��)

		// �I�v�V�������
		for(int i=2; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'v':
				case 'V':
					fVerbose=true;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		
		// ���̓t�@�C���I�[�v��
		hInFile = CreateFile(infile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
		if(hInFile==INVALID_HANDLE_VALUE) {
			printf("Failed to open input file '%s'\n", infile);
			exit(1);
		}
		if(!ReadHeader()) {
			printf("'%s' is not a XM7 file\n", infile);
			exit(1);
		}

		// �o�̓t�@�C�����𐶐�����
		// �t�@�C����������g���q�����������B
		strcpy(outfile, filename);
		char ext[10];
		strcpy(ext, ".img");
		switch(FileType) {
		case 0:
			if(Ascii!=0)	strcpy(ext, ".bas");
			else			strcpy(ext, ".iba");
			break;
		case 1:
			strcpy(ext, ".txt");
			break;
		case 2:
			strcpy(ext, ".mot");
			break;
		}
		strcat(outfile, ext);
		if(fVerbose) printf("Output file='%s'\n", outfile);

		// �o�̓t�@�C���I�[�v��
		hOutFile = CreateFile(outfile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
		if(hOutFile==INVALID_HANDLE_VALUE) {
			printf("Failed to open output file '%s'\n", outfile);
			exit(1);
		}

		ShowHeader();

		switch(FileType) {
		case 0:
			if(Ascii==true)		DecodeBasicFile();
			else				DecodeRawFile();
			break;
		case 1:
			DecodeAsciiFile();
			break;
		case 2:
			DecodeMachineFile();
			break;
		case 9:
			// -r�I�v�V�����w�莞
//			DecodeRawFile();
			break;
		default:
			break;
		}
		CloseHandle(hInFile);
		CloseHandle(hOutFile);
	}
};


void main(int argc, char *argv[])
{
	CFMDECODE fmdecode;
	fmdecode.Ignition(argc, argv);
}
