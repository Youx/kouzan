#ifndef __UTILS_H__
#define __UTILS_H__

#define MIN(a, b) \
	(a < b ? a : b)
#define MAX(a, b) \
	(a > b ? a : b)
#define MOD(x, mod) \
	((x%mod) < 0 ? (x%mod)+mod : x%mod)

char *b36enc(int val);

#endif
