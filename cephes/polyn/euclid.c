/*							euclid.c
 *
 *	Rational arithmetic routines
 *
 *
 *
 * SYNOPSIS:
 *
 * 
 * typedef struct
 *      {
 *      double n;  numerator
 *      double d;  denominator
 *      }fract;
 *
 * radd( a, b, c )      c = b + a
 * rsub( a, b, c )      c = b - a
 * rmul( a, b, c )      c = b * a
 * rdiv( a, b, c )      c = b / a
 * euclid( &n, &d )     Reduce n/d to lowest terms,
 *                      return greatest common divisor.
 *
 * Arguments of the routines are pointers to the structures.
 * The double precision numbers are assumed, without checking,
 * to be integer valued.  Overflow conditions are reported.
 */
 

#include "mconf.h"
#ifdef ANSIPROT
extern double torch_cephes_fabs ( double );
extern double torch_cephes_floor ( double );
double torch_cephes_euclid( double *, double * );
#else
double torch_cephes_fabs(), torch_cephes_floor(), torch_cephes_euclid();
#endif

extern double torch_cephes_MACHEP;
#define BIG (1.0/torch_cephes_MACHEP)

typedef struct
	{
	double n; /* numerator */
	double d; /* denominator */
	}fract;

/* Add fractions. */

void torch_cephes_radd( f1, f2, f3 )
fract *f1, *f2, *f3;
{
double gcd, d1, d2, gcn, n1, n2;

n1 = f1->n;
d1 = f1->d;
n2 = f2->n;
d2 = f2->d;
if( n1 == 0.0 )
	{
	f3->n = n2;
	f3->d = d2;
	return;
	}
if( n2 == 0.0 )
	{
	f3->n = n1;
	f3->d = d1;
	return;
	}

gcd = torch_cephes_euclid( &d1, &d2 ); /* common divisors of denominators */
gcn = torch_cephes_euclid( &n1, &n2 ); /* common divisors of numerators */
/* Note, factoring the numerators
 * makes overflow slightly less likely.
 */
f3->n = ( n1 * d2 + n2 * d1) * gcn;
f3->d = d1 * d2 * gcd;
torch_cephes_euclid( &f3->n, &f3->d );
}


/* Subtract fractions. */

void torch_cephes_rsub( f1, f2, f3 )
fract *f1, *f2, *f3;
{
double gcd, d1, d2, gcn, n1, n2;

n1 = f1->n;
d1 = f1->d;
n2 = f2->n;
d2 = f2->d;
if( n1 == 0.0 )
	{
	f3->n = n2;
	f3->d = d2;
	return;
	}
if( n2 == 0.0 )
	{
	f3->n = -n1;
	f3->d = d1;
	return;
	}

gcd = torch_cephes_euclid( &d1, &d2 );
gcn = torch_cephes_euclid( &n1, &n2 );
f3->n = (n2 * d1 - n1 * d2) * gcn;
f3->d = d1 * d2 * gcd;
torch_cephes_euclid( &f3->n, &f3->d );
}




/* Multiply fractions. */

void torch_cephes_rmul( ff1, ff2, ff3 )
fract *ff1, *ff2, *ff3;
{
double d1, d2, n1, n2;

n1 = ff1->n;
d1 = ff1->d;
n2 = ff2->n;
d2 = ff2->d;

if( (n1 == 0.0) || (n2 == 0.0) )
	{
	ff3->n = 0.0;
	ff3->d = 1.0;
	return;
	}
torch_cephes_euclid( &n1, &d2 ); /* cross cancel common divisors */
torch_cephes_euclid( &n2, &d1 );
ff3->n = n1 * n2;
ff3->d = d1 * d2;
/* Report overflow. */
if( (torch_cephes_fabs(ff3->n) >= BIG) || (torch_cephes_fabs(ff3->d) >= BIG) )
	{
	torch_cephes_mtherr( "rmul", OVERFLOW );
	return;
	}
/* euclid( &ff3->n, &ff3->d );*/
}



/* Divide fractions. */

void torch_cephes_rdiv( ff1, ff2, ff3 )
fract *ff1, *ff2, *ff3;
{
double d1, d2, n1, n2;

n1 = ff1->d;	/* Invert ff1, then multiply */
d1 = ff1->n;
if( d1 < 0.0 )
	{ /* keep denominator positive */
	n1 = -n1;
	d1 = -d1;
	}
n2 = ff2->n;
d2 = ff2->d;
if( (n1 == 0.0) || (n2 == 0.0) )
	{
	ff3->n = 0.0;
	ff3->d = 1.0;
	return;
	}

torch_cephes_euclid( &n1, &d2 ); /* cross cancel any common divisors */
torch_cephes_euclid( &n2, &d1 );
ff3->n = n1 * n2;
ff3->d = d1 * d2;
/* Report overflow. */
if( (torch_cephes_fabs(ff3->n) >= BIG) || (torch_cephes_fabs(ff3->d) >= BIG) )
	{
	torch_cephes_mtherr( "rdiv", OVERFLOW );
	return;
	}
/* euclid( &ff3->n, &ff3->d );*/
}





/* Euclidean algorithm
 *   reduces fraction to lowest terms,
 *   returns greatest common divisor.
 */


double torch_cephes_euclid( num, den )
double *num, *den;
{
double n, d, q, r;

n = *num; /* Numerator. */
d = *den; /* Denominator. */

/* Make numbers positive, locally. */
if( n < 0.0 )
	n = -n;
if( d < 0.0 )
	d = -d;

/* Abort if numbers are too big for integer arithmetic. */
if( (n >= BIG) || (d >= BIG) )
	{
	torch_cephes_mtherr( "euclid", OVERFLOW );
	return(1.0);
	}

/* Divide by zero, gcd = 1. */
if(d == 0.0)
	return( 1.0 );

/* Zero. Return 0/1, gcd = denominator. */
if(n == 0.0)
	{
/*
	if( *den < 0.0 )
		*den = -1.0;
	else
		*den = 1.0;
*/
	*den = 1.0;
	return( d );
	}

while( d > 0.5 )
	{
/* Find integer part of n divided by d. */
	q = torch_cephes_floor( n/d );
/* Find remainder after dividing n by d. */
	r = n - d * q;
/* The next fraction is d/r. */
	n = d;
	d = r;
	}

if( n < 0.0 )
	torch_cephes_mtherr( "euclid", UNDERFLOW );

*num /= n;
*den /= n;
return( n );
}
