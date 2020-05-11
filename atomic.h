#ifndef GATEWAY_ATOMIC_H_
#define GATEWAY_ATOMIC_H_

int atomic_int_get(const volatile int *atomic);

void atomic_int_set (volatile int *atomic,int value);
                    
void atomic_int_inc(volatile int *atomic);

bool atomic_int_dec_and_test(volatile int *atomic);

bool atomic_int_compare_and_exchange(
															volatile int *atomic,
															int           oldval,
															int           newval);
#if 0                                        
int atomic_int_add (volatile int *atomic,int val);

                 
unsigned int atomic_int_and(
														volatile unsigned int *atomic,
                    				unsigned int           val);

unsigned int atomic_int_or (
										volatile unsigned int *atomic,
                   	unsigned int           val);
                   
unsigned int atomic_int_xor(volatile unsigned int *atomic,
                    unsigned int           val);
                        
int
atomic_int_exchange_and_add (volatile int *atomic,
                               int           val);
#endif

#endif
