#ifndef _FMERR_H_
#define _FMERR_H_

#define FM_NO_ERROR				(0)
#define FM_FILE_NOT_FOUND		(1)
#define FM_NO_DISK_SPACE		(2)
#define FM_NOT_A_FBASIC_DISK	(3)

const char ErrString[][80] = {
	"NO ERROR",
	"File not found",
	"No disk space",
	"Not a F-BASIC Disk"
};

#endif // _FMERR_H_
