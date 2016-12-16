#ifndef _AR_H
#define	_AR_H		1

#define		ARMAG		"!<arch>\n"	
#define		SARMAG		8		

#define		AR_EFMT1	"#1/"		
#define		ARFMAG		"`\n"

struct ar_hdr 
{
		char ar_name[16];		///< name 
		char ar_date[12];		///< modification date 
		char ar_uid[6];			///< uid 
		char ar_gid[6];			///< gid 
		char ar_mode[8];		///< permissions 
		char ar_size[10];		///< size 
		char ar_fmag[2];		///< checksum 
};

#endif
