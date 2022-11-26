#ifndef A27_UTILS__H
#define A27_UTILS__H

#include "a27.h"
unsigned char A27DeriveChallenge(unsigned char inval);
void A27SetChecksum(struct A27_Read_Message* msg);

#endif /* A27_UTILS__H */