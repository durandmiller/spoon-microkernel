
double log(double x)
{
	double ans;

		asm ( "fldln2; fxch; fyl2x " : "=t" (ans) : "0" (x) : "st(1)" ); 

	return ans;
}


