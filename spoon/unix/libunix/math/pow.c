#include <math.h>

/** Logarithm method:
 *
 *
 *  a = x^y
 *
 *   =>
 *  
 *  y = log - base x - of a
 *  y = log(a)/log(x)
 *  y * log(x) = log(a)
 *
 *  =>
 *
 *  e^(y * log(x)) = a
 *
 */


double pow(double x, double y)
{
	return exp( y * log( x ) );
}

	   
