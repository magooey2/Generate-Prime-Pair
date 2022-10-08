/**********************************************************************
 * gen_pair_pseudo.c -- Create pseudo primes.  Algorithm informed by
 *                      FIPS 186-3, especially pp. 50-53, 55-56.
 *                      Sections B.3.1, B.3.3.  Based on GMP software
 *                      package.
 *
 * Copyright 2022 Jesse I. Deutsch
 *
 * Remark -- Public exponent e must be between 2^{16} and 2^{256},
 *           so it is bounded below by 65536 and has an upper bound 
 *           of about 70 decimal digits.
 *
 *           We use GMP functions for random bit and random number 
 *           generation instead of SHA-nnn hashes.
 *
 * $Id: gen_pair_pseudo.c,v 1.4 2022/10/07 03:47:20 jdeutsch Exp $
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <gmp.h>


     /******** #defines and typedefs  ********/
typedef int      BOOL;
#define NUMTESTS (50)


     /******** globals in this file   ********/
char            *program_name;      /* name of the program (for errors) */
gmp_randstate_t  rndState;


     /******** functions in this file ********/
BOOL  fnCreate_pseudo_prime (mpz_t mpzPrime, mpz_t mpzE, mpz_t mpzCompare, \
      int nNumBits, int nNumTests, BOOL flTestDiff );
BOOL  fnCompute_exponent_d (mpz_t mpzP1, mpz_t mpzE, mpz_t mpzP2, \
      mpz_t mpzD, int nNumBits);
BOOL  fnGet_key_length (int *pnNumBits);
BOOL  fnGet_rand_seed (int *pnSeed);
BOOL  fnGet_exponent_e (mpz_t mpzE);



/*************** main -- entry point **********************/
int main()
{
  int     nBitLen;                         /* number of bits           */
  int     nHalfLen;                        /* bit length / 2           */
  mpz_t   mpzP1, mpzP2, mpzE, mpzD, t;     /* two primes P, exponent E */
  mpz_t   mpzBoundE;                       /* upper bound for E        */
  int     nSeed;                           /* seed of random generator */


  /* 1. Get the key length */
  fnGet_key_length (&nBitLen);
  nHalfLen = nBitLen / 2;

  /* 2. Initialize the numbers */
  mpz_inits(mpzP1, mpzP2, mpzE, mpzBoundE, mpzD, t, NULL);
  mpz_set_ui(mpzP1,0);
  mpz_set_ui(mpzP2,0);
  mpz_set_ui(mpzE, 1);
  mpz_set_ui(mpzD, 1);
  mpz_set_ui(mpzBoundE, 1);
  mpz_set_ui(t,1);

  /* 3. Set up random number generator */
  gmp_randinit_default (rndState);          /* initialize random state */
  fnGet_rand_seed (&nSeed);
#ifdef DEBUG06
  printf ("\t Random seed:  %d\n\n", nSeed);
#endif
  gmp_randseed_ui (rndState, nSeed);        /* use something to give randomness */

  /* 4. Produce public exponent e */
  fnGet_exponent_e (mpzE);
  										   
  printf ("  The exponent e is: ");
  mpz_out_str(stdout, 10, mpzE);
  printf ("\n");
  
  /* 5. Produce first pseudo random prime of bit length n/2 */
  fnCreate_pseudo_prime (mpzP1, mpzE, mpzP2, nHalfLen, NUMTESTS, 0);

  printf ("  The first pseudo-prime is:  ");
  mpz_out_str(stdout, 10, mpzP1);
  printf ("\n");
  printf ("  In binary it is:     ");
  mpz_out_str(stdout, 2, mpzP1);
  printf ("\n");


  /* 6. Produce second pseudo random prime of bit length n/2 */
  fnCreate_pseudo_prime (mpzP2, mpzE, mpzP1, nHalfLen, NUMTESTS, 1);

  printf ("  The second pseudo-prime is: ");
  mpz_out_str(stdout, 10, mpzP2);
  printf ("\n");
  printf ("  In binary it is:     ");
  mpz_out_str(stdout, 2, mpzP2);
  printf ("\n");

  /* 7. Find the exponent d  */
  fnCompute_exponent_d (mpzP1, mpzE, mpzP2, mpzD, nHalfLen);

  /* 8. Print the exponent d */
  printf ("  The exponent d is:          ");
  mpz_out_str(stdout, 10, mpzD);
  printf ("\n");

  /* 9. Clean up the mpz_t handles or else we will leak memory */
  mpz_clears(mpzP1, mpzP2, mpzE, mpzBoundE, mpzD, t, NULL);
  gmp_randclear (rndState);
  
  return 0;
}



/************************************************************************
 * fnCreate_pseudo_prime -- Create a pseudo prime with the requisite
 *                          number of bits.  See FIPS 186-3 p. 55.
 *
 * Remark - Do the check for exponent D later.  See top of p. 53.
 ***********************************************************************/
BOOL fnCreate_pseudo_prime (mpz_t mpzPrime, mpz_t mpzE, mpz_t mpzCompare, \
     int nNumBits, int nNumTests, BOOL flTestDiff )
{
  mpz_t   n, nSq,  mpzOneShifted;      /* n, n squared, and 1 shifted */
  mpz_t   temp;
  int     i = 0;                       /* number of iterations */
  int     retval;                      /* return value         */
  int     j;


  /* 1. Initialize the numbers */
  mpz_inits(n, nSq, temp, mpzOneShifted, NULL);


  /* 2. Produce pseudo random prime of bit length n            */
  /*    Variable mpzOneShifted contains 1 shifted to the left  */
  /*    Re-set at the top of the loop each time.               */

  while (1) {
                                  /* line 4.2 */                              
                      /* Reset variables each time through the loop. */
                      /* Set up large int with proper nmbr of bits . */
    mpz_set_ui(n,0);
    mpz_set_ui(nSq,0);
    mpz_set_ui(temp,0);
    mpz_set_ui(mpzOneShifted, 1);

    mpz_mul_2exp (mpzOneShifted, mpzOneShifted, nNumBits - 1); 
    mpz_urandomb (n, rndState, nNumBits - 1);
#ifdef DEBUG01
    printf ("   ### nNumBits: %d \n", nNumBits);
    printf ("   ### The value of n:      ");
    mpz_out_str(stdout, 10, n);
    printf ("\n");
    printf ("   ### In binary it is:     ");
    mpz_out_str(stdout, 2, n);
    printf ("\n");
#endif  

    mpz_add (n, n, mpzOneShifted);
                                  /* line 4.3,  add 1 if n not odd */
    if (mpz_odd_p (n) == 0)
      mpz_add_ui (n, n, 1L);
                                  /* line 5.4 for Second Prime only */
                                  /* check size of difference       */
    if (flTestDiff == 1) {
      mpz_sub (temp, n, mpzCompare);
      mpz_abs(temp, temp); 
                                  /* reset, new shift */
      mpz_set_ui(mpzOneShifted,1);
      j = nNumBits <= 100 ? 0 : nNumBits - 100;
      mpz_mul_2exp (mpzOneShifted, mpzOneShifted, j); 
      if (mpz_cmp (temp, mpzOneShifted) <= 0)
        continue;         
    }
      
                                  /* line 4.4, compare */
    mpz_mul (nSq, n, n);
#ifdef DEBUG02
    printf ("   ### The value of n:      ");
    mpz_out_str(stdout, 10, n);
    printf ("\n");
    printf ("   ### In binary it is:     ");
    mpz_out_str(stdout, 2, n);
    printf ("\n");
#endif
	                              /* reset, new shift  */
    mpz_set_ui(mpzOneShifted,1);
    mpz_mul_2exp (mpzOneShifted, mpzOneShifted, 2 * nNumBits - 1); 
                                  /* compare to bounds for p */
    if (mpz_cmp (nSq, mpzOneShifted) < 0)
	  continue;
                                  /* line 4.5          */
    mpz_set_ui (temp, 0);
    mpz_sub_ui (temp, n, 1L);
	mpz_gcd (temp, temp, mpzE);
	                              /* check for relatively prime */
	if (mpz_cmp_ui (temp, 1) == 0) {
                                  /* line 4.5.1 */
      retval = mpz_probab_prime_p (n, nNumTests);
                                  /* prob prime or prime */
      if (retval >= 1) 
        break;                      
    }


    i++;
    if (i >= 5 * nNumBits) {
	  printf ("   ### FAILURE creating prime\n");
	  exit(1);
	}  
  }
  
  /* 3. copy over results to return them */
  mpz_set (mpzPrime, n);    
#ifdef DEBUG03
  printf ("   ### The value of n:      ");
  mpz_out_str(stdout, 10, n);
  printf ("\n");
  printf ("   ### In binary it is:     ");
  mpz_out_str(stdout, 2, n);
  printf ("\n");
#endif
  
  /* 4. Clean up the mpz_t handles or else we will leak memory */
  mpz_clears(n, nSq, temp, mpzOneShifted, NULL);
  
  return 0;
}



/************************************************************************
 * fnCompute_exponent_d -- Find mpdD and check the size.  
 *
 * Remark - Do the check for exponent D here.  See top of p. 53.
 ***********************************************************************/
BOOL fnCompute_exponent_d (mpz_t mpzP1, mpz_t mpzE, mpz_t mpzP2, \
     mpz_t mpzD, int nNumBits)
{
  mpz_t   n, mpzOneShifted;            /* n and 1 shifted      */
  mpz_t   temp;
  int     retval;                      /* return value         */


  mpz_inits(n, temp, mpzOneShifted, NULL);
  mpz_set_ui(n,0);
  mpz_set_ui(temp,0);
  mpz_set_ui(mpzOneShifted, 1);


  /* 1. Compute the exponent D and check the size,      */
  /*    but only if we are working on the second prime. */
  /*    (p - 1) * (q - 1) = p*q - p - q + 1.            */
  mpz_mul (temp, mpzP1, mpzP2);
  mpz_sub (temp, temp, mpzP1);
  mpz_sub (temp, temp, mpzP2);
  mpz_add_ui (temp, temp, 1);
  
  retval = mpz_invert (n, mpzE, temp);
#ifdef DEBUG05
  printf ("      ### The value of d:      ");
  mpz_out_str(stdout, 10, n);
  printf ("\n");
  printf ("      ### The value of mpzP1:      ");
  mpz_out_str(stdout, 10, mpzP1);
  printf ("\n");
  printf ("      ### The value of temp:      ");
  mpz_out_str(stdout, 10, temp);
  printf ("\n");
#endif

  if (retval == 0) {
    printf ("   ### ERROR: Exponent not relatively prime to modulus\n\n");
    exit(1);
  }

  mpz_set_ui(mpzOneShifted, 1);
  mpz_mul_2exp (mpzOneShifted, mpzOneShifted, nNumBits); 
                                  /* compare to bound 2^(nlen / 2) */
  if (mpz_cmp (n, mpzOneShifted) < 0)
    fprintf (stderr, "   ### WARNING: Exponent too small\n\n");

  /* 2. copy over results to return them */
  mpz_set (mpzD, n);    

  /* 3. Clean up the mpz_t handles or else we will leak memory */
  mpz_clears(n, temp, mpzOneShifted, NULL);
  
  return 0;
}
     


/************************************************************************
 * fnGet_key_length -- Retrieve the length of the key.  
 *
 * Remark - 
 ***********************************************************************/
BOOL fnGet_key_length (int *pnNumBits)
{
  char    line[1025];


  printf ("Recommended key sizes are 2048 or 3072 for pseudo primes\n");
  printf ("Enter an even key size (nlen): ");
  fflush (stdout);                        /* needed for MinGW */
  fgets(line, sizeof(line), stdin);
  sscanf(line, "%d", pnNumBits);

  return (0);
}



/************************************************************************
 * fnGet_rand_seed -- The seed to begin random number generation.  
 *
 * Remark - 
 ***********************************************************************/
BOOL fnGet_rand_seed (int *pnSeed)
{
  char    line[129];
  char    chIn;
  time_t  nNumSecs;                        /* number of seconds        */


  printf ("\n  --> Options for the random seed. <--\n");
  printf ("      Choose Y to type an integer\n");
  printf ("      or N to use the current time: ");
  fflush (stdout);                        /* needed for MinGW */
  fgets(line, sizeof(line), stdin);
  sscanf(line, "%c", &chIn);

  if (chIn == 'Y' || chIn == 'y') {
    printf ("\t Enter the seed: ");
    fflush (stdout);                        /* needed for MinGW */
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%d", pnSeed);
  }  
  else {  
    time (&nNumSecs); 
    *pnSeed = nNumSecs;
  }

  return (0);
}



/************************************************************************
 * fnGet_exponent_e -- Get or Create the RSA key e.  
 *
 * Remark - 
 ***********************************************************************/
BOOL fnGet_exponent_e (mpz_t mpzE)
{
  char           line[129];
  char           chIn;
  mpz_t          mpzBoundE;                /* upper bound for E        */
  mpz_t          temp;
  unsigned long  nValE;                    /* value of E as a long     */

  /* 1. Initialize the numbers */
  mpz_inits(mpzBoundE, temp, NULL);
  mpz_set_ui(temp,0);
  mpz_set_ui(mpzBoundE, 1);

  /* 2. Options for exponent */
  printf ("\n  --> Options for the exponent e. <--\n");
  printf ("      Choose Y to type an integer\n");
  printf ("      or N to calculate a random number: ");
  fflush (stdout);                        /* needed for MinGW */
  fgets(line, sizeof(line), stdin);
  sscanf(line, "%c", &chIn);

  if (chIn == 'Y' || chIn == 'y') {
    printf ("\t Enter the value of e (often used are 3, 5, 17, 257, 65537: ");
    fflush (stdout);                        /* needed for MinGW */
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%lu", &nValE);
                                            /* copy results for return */
    mpz_set_ui (mpzE, (unsigned long) nValE);
  }  
  else {  
                                            /* set upper bound for E */
    mpz_mul_2exp (mpzBoundE, mpzBoundE, 256); 
    while (1) {
      mpz_urandomb (temp, rndState, 256);
                                           /* if even, try again */
      if (mpz_even_p (temp) != 0) 
	    continue;
	                                       /* compare to bounds for E */
      if (mpz_cmp_ui (temp, 65536) < 0)
	    continue;
      else if (mpz_cmp (temp, mpzBoundE) > 0)
	    continue;
      else
        break;
    }	  
                                            /* copy results for return */
  mpz_set (mpzE, temp);    
  }

  mpz_clears(mpzBoundE, temp, NULL);

  return (0);
}

