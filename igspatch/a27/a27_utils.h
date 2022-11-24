#pragma once

#include "a27.h"
unsigned char A27DeriveChallenge(unsigned char inval);
void A27SetChecksum(struct A27_Read_Message* msg);

