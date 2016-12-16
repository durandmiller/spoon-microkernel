#ifndef _BITS_SIGNAL_H
#define _BITS_SIGNAL_H

// sigev_notify options
#define	SIGEV_NONE			1		// No asynchronous notification  
#define	SIGEV_SIGNAL		2		// A queued signal will be generated
#define	SIGEV_THREAD		3		// A notification function will be called


// Realtime Signals Extension
#define SIGRTMIN 0
#define SIGRTMAX 0
#define RTSIG_MAX 0




// --

#define SIGABRT			1
#define SIGALRM			2
#define SIGFPE			3
#define SIGHUP			4
#define SIGILL			5
#define SIGINT			6
#define SIGKILL			7
#define SIGPIPE			8
#define SIGQUIT			9
#define SIGSEGV			10
#define SIGTERM			11
#define SIGUSR1			12
#define SIGUSR2			13
#define SIGCHLD			14
#define SIGCONT			15
#define SIGSTOP			16
#define SIGTSTP			17
#define SIGTTIN			18
#define SIGTTOU			19
#define SIGBUS			20
#define SIGPOLL			21
#define SIGPROF			22
#define SIGSYS			23
#define SIGTRAP			24
#define SIGURG			25
#define SIGVTALRM		26
#define SIGXCPU			27
#define SIGXFSZ			28




#define		SA_NOCLDSTOP			1
#define		SIG_BLOCK				0
#define		SIG_UNBLOCK				1
#define		SIG_SETMASK				2
#define		SA_ONSTACK				0x08000000
#define		SA_RESETHAND			0x80000000 
#define		SA_RESTART				0x10000000
#define		SA_SIGINFO				4
#define		SA_NOCLDWAIT			2
#define		SA_NODEFER				0x40000000
#define		SS_ONSTACK				1
#define		SS_DISABLE				2
#define		MINSIGSTKSZ				2048
#define		SIGSTKSZ				8192




#define		ILL_ILLOPC 		1	//	illegal opcode
#define		ILL_ILLOPN		2	// 	illegal operand
#define		ILL_ILLADR		3	// 	illegal addressing mode
#define		ILL_ILLTRP		4	// 	illegal trap
#define		ILL_PRVOPC		5	// 	privileged opcode
#define		ILL_PRVREG		6	// 	privileged register
#define		ILL_COPROC		7	// 	coprocessor error
#define		ILL_BADSTK		8	// 	internal stack error
#define		FPE_INTDIV		1	//	integer divide by zero
#define		FPE_INTOVF		2	// 	integer overflow
#define		FPE_FLTDIV		3	// 	floating point divide by zero
#define		FPE_FLTOVF		4	// 	floating point overflow
#define		FPE_FLTUND		5	// 	floating point underflow
#define		FPE_FLTRES		6	// 	floating point inexact result
#define		FPE_FLTINV		7	// 	invalid floating point operation
#define		FPE_FLTSUB		8	// 	subscript out of range
#define		SEGV_MAPERR		1	// 	address not mapped to object
#define		SEGV_ACCERR		2	// 	invalid permissions for mapped object
#define		BUS_ADRALN		1	//	invalid address alignment
#define		BUS_ADRERR		2	// 	non-existent physical address
#define		BUS_OBJERR		3	// 	object specific hardware error
#define		TRAP_BRKPT		1	// 	process breakpoint
#define		TRAP_TRACE		2	// 	process trace trap
#define		CLD_EXITED		1	//	child has exited
#define		CLD_KILLED		2	// 	child has terminated abnormally, core
#define		CLD_DUMPED		3	// 	child has terminated abnormally, core
#define		CLD_TRAPPED		4	// 	traced child has trapped
#define		CLD_STOPPED		5	// 	child has stopped
#define		CLD_CONTINUED	6	// 	stopped child has continued
#define		POLL_IN 		1	//	data input available
#define		POLL_OUT		2	// 	output buffers available
#define		POLL_MSG		3	// 	input message available
#define		POLL_ERR		4	// 	I/O error
#define		POLL_PRI		5	// 	high priority input available
#define		POLL_HUP		6	// 	device disconnected
#define		SI_USER			1	// 	signal sent by kill()
#define		SI_QUEUE		2	// 	signal sent by the sigqueue()
#define		SI_TIMER		3	// 	expiration of a timer set by timer_settime()
#define		SI_ASYNCIO		4	// 	completion of an asynchronous I/O request
#define		SI_MESGQ		5	// 	message on an empty message queue







#endif

