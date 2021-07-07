#ifndef BNPRINT_H
#define BNPRINT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct BigNum;

#ifndef SWIG
int bnPrint(FILE *f, char const *prefix, struct BigNum const *bn,
	char const *suffix);
#endif

/**
 * Convert an ASCII string into a BigNum.
 *
 * This function converts an ASCII string into a Big number. If the first
 * character of the string is a minus sign the big number is a negative number.
 *
 * @param X the BigNum that stores the result
 *
 * @param s the ASCII string in big-endian format (first digit is most significant)
 *
 * @param radix the function can use radix between 2 and 16
 */
int bnReadAscii(struct BigNum *X, char *s, int radix);
#ifdef __cplusplus
}
#endif

#endif /* BNPRINT_H */
