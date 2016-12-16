							  
							  
double exp(double x)
{
	double ans;


	asm (  
			"fldl2e;"
			"fmulp %%st, %%st(1);"
			"fst %%st(1);"
			"frndint;"
			"fxch;"
			"fsub %%st(1);"
			"f2xm1;"
			"fld1;"
			"faddp;"
			"fscale;"
			"fstp %%st(1);"

			: "=t" (ans)
			: "0" (x)
		);


	return ans;
}


