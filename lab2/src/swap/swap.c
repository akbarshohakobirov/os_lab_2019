#include "swap.h"

void Swap(char *left, char *right)
{
	char n;
	n = *(left);
	*left = *right;
    *right = n;
}
