#include <inttypes.h>


double ceil(double x)
{
	double ans;

	uint16_t	ctrlWord1;
	uint16_t	ctrlWord2 = 0xb63;


		asm ( "fstcw %0 " : "=m" (ctrlWord1) ); 
		asm ( "fclex " );
		asm ( "fldcw %0 " : : "m" (ctrlWord2) ); 
		asm ( "frndint" : "=t" (ans) : "0" (x) );
		asm ( "fclex " );
		asm ( "fldcw %0 " : : "m" (ctrlWord1) ); 

	return ans;
}


