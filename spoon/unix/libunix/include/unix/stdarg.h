#ifndef _STDARG_H
#define _STDARG_H		1

/* We're going to use the GCC built-in macros if we're being compiled with gcc. */


#ifdef __GNUC__
typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)

#else 


#ifndef _HAVE_VA_LIST
#define _HAVE_VA_LIST
typedef struct 
{
	void **start;
	void **ptr;
} va_list;
#endif


#define		va_start( ap, lastarg )					\
			ap.ptr = (void**)( (uintptr_t)&lastarg + sizeof(lastarg) );	\
			ap.start = ap.ptr;	


#define		va_arg( ap, type) \
			((type)*(ap.ptr)); \
			ap.ptr = (void**)( ((uintptr_t)ap.ptr) + sizeof( type ) );
		
#define		va_end( ap )	\
			ap.ptr = NULL;

#endif


#define VA_START	va_start
#define VA_END		va_end
#define VA_ARG		va_arg

#endif

