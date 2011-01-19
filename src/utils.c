#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

char *b36enc(int val)
{
	char *lookup = "0123456789abcdefghijklmnopqrstuvwxyz";
	int val2 = abs(val);
	int i;
	char *res = calloc(8, sizeof(char)); // int = 6 b36 digits;
	char *ptr = res;

	if (val == 0) {
		res[0] = '0';
		return res;
	}
	while (val2 != 0) {
		i = val2 % 36;
		val2 = val2 / 36;
		*ptr = lookup[i];
		ptr++;
	}
	/* minus sign */
	if (val < 0) {
		*ptr = '-';
		ptr++;
	}
	/* now reverse the string */
	int len = ptr - res;
	for (i = 0 ; i < len/2 ; i++) {
		char tmp = res[i];
		res[i] = res[len-i-1];
		res[len-i-1] = tmp;
	}
	//printf("%i => %s\n", val, res);
	return res;
}
