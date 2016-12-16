
double log10(double x)
{
	double ans;

		asm ( "fldlg2; fxch; fyl2x " : "=t" (ans) : "0" (x) : "st(1)" ); 

	return ans;
}


