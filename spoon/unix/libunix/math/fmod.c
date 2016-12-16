							  
							  
double fmod(double x, double y)
{
	double ans;


		asm ( "1: fprem;"
		      "fnstsw  %%ax;"
			  "test $0x400, %%ax;"
			  "jnz 1"
			  : "=t" (ans) 
			  : "0" (x), "u" (y) 
			  : "ax" ); 

	return ans;
}


