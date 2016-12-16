#ifndef _ALLOCA_H
#define _ALLOCA_H			1

#ifdef  __GNUC__

#define alloca( size )   __builtin_alloca( size )

#endif

#endif
