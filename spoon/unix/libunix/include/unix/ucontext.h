#ifndef _UCONTEXT_H
#define _UCONTEXT_H			1


#include <sys/types.h>


#ifndef _HAVE_MCONTEXT_T
#define _HAVE_MCONTEXT_T
typedef struct
{
	
} mcontext_t;
#endif

#ifndef _HAVE_SIGSET_T
#define _HAVE_SIGSET_T
typedef struct
{
	unsigned long bits[ (1024 / sizeof( unsigned long )) + 1 ];
} sigset_t;
#endif



#ifndef _HAVE_STACK_T
#define _HAVE_STACK_T
typedef struct 
{
	void     *ss_sp;
	size_t    ss_size;
	int       ss_flags;
} stack_t;
#endif


#ifndef _HAVE_UCONTEXT_T
#define _HAVE_UCONTEXT_T
typedef struct ucontext
{
	struct ucontext	*uc_link;	// pointer to the context that will be resumed
	sigset_t	uc_sigmask;		// the set of blocked signals
	stack_t     uc_stack;		// the stack used by this context
	mcontext_t  uc_mcontext;	// a machine-specific representation
}	ucontext_t;
#endif









int  getcontext(ucontext_t *);
int  setcontext(const ucontext_t *);
void makecontext(ucontext_t *, void (*func)(), int, ...);
int  swapcontext(ucontext_t *, const ucontext_t *);

#endif

