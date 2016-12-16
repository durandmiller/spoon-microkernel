#ifndef _FNMATCH_H
#define _FNMATCH_H		1



#define	FNM_NOMATCH		1
#define FNM_PATHNAME	(1<<0)
#define FNM_NOESCAPE	(1<<1)
#define FNM_PERIOD		(1<<2)

#define FNM_FILE_NAME		FNM_PATHNAME
#define FNM_LEADING_DIR		(1<<3)
#define FNM_CASEFOLD		(1<<4)
#define FNM_EXTMATCH		(1<<5)

#define FNM_NOSYS			(-1)


#ifdef __cplusplus
extern "C" {
#endif
		

int fnmatch(const char *, const char *, int);

		
#ifdef __cplusplus
}
#endif
		


#endif

