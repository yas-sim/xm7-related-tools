#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "CMotorolaS.h"

// v0.1 001009	"iba"�g���q��BASIC�o�C�i���v���O�����Ƃ��Ĉ����悤�ɕύX

#define VERSION "v0.1"

class CFMENCODE {
protected:
	HANDLE hInFile, hOutFile;				// ���o�̓t�@�C���ւ̃n���h��
	char infile[512], outfile[512];			// �o�̓t�@�C����

	bool fRaw;								// RAW���[�h�t���O

	char filename[9];
	unsigned char FileType;
	bool Ascii;
	bool Random;

	bool fVerbose;
protected:
	// �t�@�C��������g���q�����݂̂𔲂����
	void GetFileExtention( char *filename, char *ext) {
		int ln = 0;
		char *ptr = filename;
		while(*ptr) ptr++;								// ������̍Ō��T�� 
		while(ptr>=filename && *ptr!='.') ptr--;		// '.'�������邩�A������̐擪�ɂ���܂ŉ�����
		if(*ptr=='.') {
			ptr++;
			while(*ptr) {
				*ext++ = *ptr++;						// '.'�����������ꍇ�Aext��ptr+1�ȍ~�𕡎�
				if(ln++>=3) break;						// �g���q���R�����𒴂����烋�[�v�𔲂���
			}
		}
		*ext = '\0';									// ��������^�[�~�l�[�g����
	}

	// �t�@�C�������̃x�[�X�t�@�C���������𔲂��o��
	void GetFileBase( char *filename, char *base) {
		int ln = 0;
		while(*filename!='\0' && *filename!='.') {
			*base++ = *filename++;
			if(ln++>=8) break;
		}
		*base = '\0';
	}

	// �g���q����͂��āA�t�@�C���^�C�v�����肷��
	bool AnalyzeFileExtention( char *ext, unsigned char *FileType, bool *Ascii, bool *Random) {
		// �f�t�H���g�t�@�C���^�C�v��ݒ肵�Ă���
		*FileType = 1;
		*Ascii = true;
		*Random = false;
		if(strlen(ext)<=3) {
			// �g���q�̒������R�ȉ��̂Ƃ��̂݉�͂���
			_strupr(ext);
			if(strcmp(ext, "TXT")==0) {
				*FileType	= 1;
				*Ascii		= true;
			}
			if(strcmp(ext, "MOT")==0 || strcmp(ext,"S")==0) {
				*FileType	= 2;
				*Ascii		= false;
			}
			if(strcmp(ext, "BAS")==0) {
				*FileType	= 0;
				*Ascii		= true;
			}
			if(strcmp(ext, "IBA")==0) {
				*FileType	= 0;
				*Ascii		= false;
			}
		} else return false;
		return true;
	}

	void EncodeBasicFile( void ) {
		unsigned char buff[4];
		DWORD NOR, NOW;
		buff[0]=0x0d;
		buff[1]=0x0a;
		WriteFile(hOutFile, buff, 2, &NOW, NULL);
		while(1) {
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) break;
			WriteFile(hOutFile, buff, 1, &NOW, NULL);
		}
		buff[0] = 0x0d;
		buff[1] = 0x0a;
		buff[2] = 0x1a;
		WriteFile(hOutFile, buff, 3, &NOW, NULL);
	}

	// �ׂ�������
	void EncodeRawFile( void ) {
		unsigned char buff[4];
		DWORD NOR, NOW;
		while(1) {
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) break;
			WriteFile(hOutFile, buff, 1, &NOW, NULL);
		}
	}

	void EncodeDataFile( void ) {
		EncodeBasicFile();			// ���������ł��܂�
	}

	
	bool ReadLine( unsigned char *ptr, int max ) {
		DWORD NOR;
		char buff[4];
		int linecount=0;
		while(1) {
			*ptr = '\0';
			if(linecount>=max) return false;
			ReadFile(hInFile, buff, 1, &NOR, NULL);
			if(NOR==0) {
				if(linecount==0) return false;
				return true;
			}
			if(buff[0]<' ') break;
			*ptr++ = buff[0];
			linecount++;
		}
		return true;
	}

	void EncodeMachineFile( void )
	{
		unsigned char motbuf[0x10000];			// 64KB�m��
		unsigned long minadr, maxadr;
		unsigned char buff[512];
		CMotorolaS mot;
		minadr = 0xffff;
		maxadr = 0x0000;
		unsigned long adr;
		unsigned long byte;
		unsigned long entry;
		while(ReadLine(buff, 510)) {
			if(buff[0]!='S') continue;
			mot.SetSRecord((char*)buff);				// S���R�[�h���Z�b�g
			adr  = mot.GetLoadAddress();				// ���[�h�A�h���X���擾
			byte = mot.GetDataCount();					// �f�[�^�����擾
			unsigned int i;
			switch(mot.GetRecordType()) {
			case 1:
			case 2:
			case 3:
				if(minadr>adr)			minadr = adr;		// �ŏ��A�h���X���L�^
				if(maxadr<adr+byte-1u)	maxadr = adr+byte-1;// �ő�A�h���X���L�^
				for(i=0; i<byte; i++) {
					motbuf[adr+i] = mot.GetData(i);			// �f�[�^��motrola-s�o�b�t�@�ɃR�s�[
//					printf("%02x", motbuf[adr+i]);
				}
//				printf("\n");
//				printf("adr=%04x size=%04x\n", adr, byte);
				break;
			case 7:
			case 8:
			case 9:
				entry = mot.GetLoadAddress();
				break;
			}
		}
//		printf("Min adrs=%04x  Max adrs=%04x  Entry=%04x\n", minadr, maxadr, entry);

		DWORD NOW;
		unsigned long size = maxadr-minadr+1;
		buff[0] = (unsigned char)0x00;
		buff[1] = (unsigned char)(size>>8);
		buff[2] = (unsigned char)(size & 0x00ff);
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// �t�@�C���T�C�Y��������
		buff[0] = (unsigned char)(minadr>>8);
		buff[1] = (unsigned char)(minadr & 0x00ff);
		WriteFile(hOutFile, buff, 2, &NOW, NULL);		// ���[�h�A�h���X��������
		WriteFile(hOutFile, motbuf+minadr, size, &NOW, NULL);	// ���f�[�^��������
		buff[0] = 0xff;
		buff[1] = 0;
		buff[2] = 0;
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// �_�~�[�R�o�C�g��������
		buff[0] = (unsigned char)(entry>>8);
		buff[1] = (unsigned char)(entry & 0x00ff);
		buff[2] = 0x1a;			// EOF
		WriteFile(hOutFile, buff, 3, &NOW, NULL);		// �G���g���[�A�h���X&EOF��������
	}

	// �w�b�_����\������
	void ShowHeader( void ) {
		if(fVerbose)
			printf("Filename=%8s FileType=%d Ascii=%c Random=%c\n", filename, FileType, Ascii?'A':'B', Random?'R':'S');
	}

	// �w�b�_������������
	void WriteHeader( void )  {
		char header[0x10];
		DWORD NOW, ptr;
		// �w�b�_��0�N���A
		for(int i=0; i<0x10; i++) header[i]=0;
		ptr=0;
		// �t�@�C��������
		for(i=0; i<9; i++) {
			header[ptr++] = filename[i];
			if(filename[i]=='\0') break;
		}
		header[10] = FileType;
		header[11] = Ascii;
		header[12] = Random;
		// ���ʗp�}�W�b�N�i���o�[
		header[13] = 'X';
		header[14] = 'M';
		header[15] = '7';
		WriteFile(hOutFile, header, 0x10, &NOW, NULL);
	}

public:

	CFMENCODE() {
		fRaw = true;
		fVerbose=false;
	}

	void Usage( void )	{
		puts("DOS/V file to FM-FILE converter " VERSION );
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: FMENCODE IN_FILE [options]");
		puts("Options: -v  Verbose mode");
		puts("ex. FMENCODE test.txt");
		puts("Note: You can specify the DOS/V file which have following file extention:");
		puts("   txt  : Text file          will convert to '1 A 0' file");
		puts("   mot,s: Motrola-S file     will convert to '2 B 0' file");
		puts("   bas  : Basic file(ASC)    will convert to '0 A 0' file");
		puts("   iba  : Basic file(i-code) will convert to '0 B 0' file");
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
					fVerbose = true;
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
			puts("Failed to open input file");
			exit(1);
		}

		// �x�[�X�t�@�C�������o��
		GetFileBase(infile, filename);

		// �g���q���o�������
		char ext[10];		// �t�@�C���g���q
		GetFileExtention(infile, ext);
		AnalyzeFileExtention(ext, &FileType, &Ascii, &Random);

		if(fVerbose) 
			printf("fn=%s ext=%s %d %c %c\n", infile, ext, FileType, Ascii==true?'A':'B', Random==true?'1':'0');

		// �o�̓t�@�C�������쐬
		sprintf(outfile, "%s.%d%c%d", filename,
			FileType,					// �t�@�C���^�C�v 0,1,2
			Ascii?'A':'B',				// �A�X�L�[�t���O
			Random?1:0);
		if(fVerbose)
			printf("FM-FILE file ='%s'\n", outfile);

		// �o�̓t�@�C���I�[�v��
		hOutFile = CreateFile(outfile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
		if(hOutFile==INVALID_HANDLE_VALUE) {
			puts("Failed to open output file");
			exit(1);
		}

		// �w�b�_������������
		WriteHeader();

		switch(FileType) {
		case 0:
			if(Ascii==true) 
				EncodeBasicFile();
			else {
				EncodeRawFile();
			}
			break;
		case 1:
			EncodeDataFile();
			break;
		case 2:
			EncodeMachineFile();
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
	CFMENCODE fmencode;
	fmencode.Ignition(argc, argv);
}
