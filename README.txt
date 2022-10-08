                    gen_pair_pseudo Primes

  The purpose of this project is to create pseudo-primes in the
spirit of FIPS 186-3 and FIPS 186-4 using GMP.  FIPS is one of
the Federal Information Processing Standards developed by the
US National Institutes of Standards and Technology.  GMP is the
GNU Multiprecision Library.  

  One of the main differences is that we do not use one of the 
prescribed hash functions to get pseudo random numbers, but 
instead depend on GMP's internal mechanism.  Consequently the
program is not quite in accord with the standard.  

  Also to be noted is that the exponent e can be chosen in
accord with the standard in a pseudo random fashion as long
as it is odd and between 2^{16} and 2^{256}.

  The user is given the option of using a fixed or pseudo random
value for e.  The user can also use a fixed or time dependent
seed for the random number generator.

---------------------------



