
.globl _start


_start:
	pushl %edx
	pushl %ecx
	pushl %edx
	pushl %eax
	call linux_start

