#----------------------------------------------------------
# GCC LINUX make file for gen_pair_pseudo primes.
#
#    Copyright 2022 Jesse I. Deutsch
#
# Remark - Type make NDEBUG=1 for no debugging version.
#
# $Id:$
#----------------------------------------------------------



#----- make NDEBUG=1 for nodebugging -----#
ifeq ($(NDEBUG), 1)
CL = gcc -O2 -c -DNDEBUG
LINK = gcc -O2 -DNDEBUG
OPT = -mmmx -msse2
PROFL = 


#----- DEBUG case is below -----#
else   
CL = gcc -g -c -Wall -Wextra -DDEBUG
LINK = gcc -g -Wall -Wextra -DDEBUG
OPT = 
PROFL = -pg
endif



#----- default for make -----#
all : gen_pair_pseudo    


#----- project is here -----#
gen_pair_pseudo : gen_pair_pseudo.o
	$(LINK) $(PROFL) -o a.out gen_pair_pseudo.o -lgmp

gen_pair_pseudo.o : gen_pair_pseudo.c
	$(CL) $(OPT) $(PROFL) gen_pair_pseudo.c


#----- cleaning of files -----#
clean :
	for f in *.o;   do rm -f $$f; done
	for f in *.bak; do rm -f $$f; done	
	for f in *.out; do rm -f $$f; done
	for f in *.exe; do rm -f $$f; done
	for f in core;  do rm -f $$f; done
	for f in *~;    do rm -f $$f; done

