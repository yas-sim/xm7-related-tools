#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include <cfdimg.h>
#include "cdump.h"

#define VERSION "v0.0"

#define SECT_SIZE			(256)	// 1�Z�N�^�̃T�C�Y
#define MAX_CLUSTER			(151)	// �N���X�^�ԍ��̍ő�l
#define SPC					(8)		// Sector/Cluster
#define BPE					(32)	// Byte/Entry
#define SPT					(16)	// Sector/Track(Cylinder)
#define HPT					(2)		// Head/Track(Cylinder)
#define CLUSTER_TOP_SECTOR	(64)	// ��O�N���X�^�̐擪�Z�N�^�ԍ�(LBA)

inline int	LBA2C( int LBA ) {
	return (LBA/(SPT*HPT));
}

inline int LBA2H( int LBA ) {
	return (LBA/SPT)&1;
}

inline int LBA2R( int LBA ) {
	return LBA-(LBA/SPT)*SPT+1;
}

inline int CHR2LBA( int C, int H, int R ) {
	return (C*HPT+H)*SPT+R-1;
}

inline int Cluster2LBA( int nCluster ) {
	return nCluster*SPC+CLUSTER_TOP_SECTOR;
}


class CD77DMP {
protected:
	CFDImg	fd;
	int track, sector;
	bool fManual;
public:

	CD77DMP() {
		fManual = false;
	}

	void Usage( void )	{
		puts("D77 Image format file dump tool " VERSION);
		puts("Programmed by an XM7 supporter\n");
		puts("Synopsis: D77DMP FDIMAGE_NAME [option]\n");
		puts("Option type1 : num1 num2  : num1=Track# num2=Sector#");
		puts("Option type2 : -Cnum      : num=Cluster#");
		puts("Option type3 : -Lnum      : num=LBA#\n");
		puts("Another option: -m        : Manual mode\n");
		puts("eg1. D77DMP disk.d77 1 1  -> Track1 Sector1");
		puts("eg2. D77DMP disk.d77 -c2  -> Cluster2");
		puts("eg3. D77DMP disk.d77 -L5  -> LBA 5\n");
		puts("Note: LBA number must be start from 0 (LBA 0 = Track0,Sector1)");
		puts("You can increse/decrese sector # manually in manual mode");
	}

	void ShowSector( int trk, int sct ) {
		CSector sect;
		CDump dump;
		size_t numRead;
		uint8_t buff[4096];
		sect.m_pSectorData = buff;
		printf("Dumping TRK=%d SCT=%d", trk, sct);
		if(fd.ReadSector(trk/2, trk%2, sct, sect, numRead)!=0) {
			puts("Failed to read sector");
			return;
		}
		for(int i=0; i<numRead; i++) {
			dump.put(buff[i]);
		}
		dump.flush();
	}

	void Ignition( int argc, char*argv[]) {
		if(argc<3) {
			Usage();
			exit(1);
		}
		
		track = -1;
		sector = -1;

		int cluster, LBA;
		// �I�v�V�������
		for(int i=2; i<argc; i++) {
			switch(argv[i][0]) {
			case '/':
			case '-':
				switch(argv[i][1]) {
				case 'h':
				case 'H':
				case '?':
					Usage();
					break;
				case 'M':
				case 'm':
					fManual=true;
					break;
				case 'C':
				case 'c':
					cluster=atoi(argv[i]+2);	// �N���X�^�ԍ��擾
					LBA=Cluster2LBA(cluster);	// �N���X�^�ԍ���LBA�ԍ��ɕϊ�
					track=LBA2C(LBA)*2+LBA2H(LBA);	// LBA�ԍ����g���b�N�ԍ��ɕϊ�
					sector=LBA2R(LBA);				// LBA�ԍ����Z�N�^�ԍ��ɕϊ�
					break;
				case 'L':
				case 'l':
					LBA=atoi(argv[i]+2);
					track=LBA2C(LBA)*2+LBA2H(LBA);	// LBA�ԍ����g���b�N�ԍ��ɕϊ�
					sector=LBA2R(LBA);				// LBA�ԍ����Z�N�^�ԍ��ɕϊ�
					break;
				default:
					Usage();
					break;
				}
				break;
			default:
				if(track==-1) {
					track=atoi(argv[i]);		// track�ԍ���-1�̂Ƃ��́A�I�v�V�������g���b�N�ԍ��Ƃ��ĉ���
				} else {
					sector=atoi(argv[i]);		// sector�ԍ���-1�̂Ƃ��́A�I�v�V�������Z�N�^�ԍ��Ƃ��ĉ���
				}
				break;
			}
		}
		if(track==-1 || sector==-1) {
			printf("Illegal parameter");
			exit(-1);
		}
		if(fd.OpenFDImage(std::string(reinterpret_cast<char*>(argv[1])))==false) {
			puts("Failed to open FD image file");
			exit(1);
		}
		if(fManual==false) {
			ShowSector(track, sector);
		} else {
			while(fManual) {
				ShowSector(track, sector);
				puts("P:Prev  N:Next  ESC,Q:Exit");
				int ch = getch();
				switch(ch) {
				case 0x1b:
				case 'q':
				case 'Q':
					fManual=false;
					break;
				case 'n':
				case 'N':
					if(++sector>16) {
						sector=1;
						if(++track>79) {
							track=79;
						}
					}
					break;
				case 'p':
				case 'P':
					if(--sector<1) {
						sector=16;
						if(--track<0) {
							track=0;
						}
					}
					break;
				}
			}
		}
		fd.CloseFDImage();
	}
};

void main(int argc, char *argv[])
{
	CD77DMP c77dmp;
	c77dmp.Ignition(argc, argv);
}
