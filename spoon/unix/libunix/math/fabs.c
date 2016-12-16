


double	fabs(double x)
{
	double ans;
	asm( "fabs" : "=t" (ans) : "0" (x) );
	return ans;
}





