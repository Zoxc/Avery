#include "common.h"

BOOTSTRAP_CODE

// Write len copies of val into dest.
void *memset(void *ptr, int value, unsigned int num)
{
	uint8_t *dest = (uint8_t *)ptr;
	for (; num != 0; num--) *dest++ = value;
	return ptr;
}

// Compare two strings. Should return -1 if
// str1 < str2, 0 if they are equal or 1 otherwise.
unsigned int strcmp(const char *str1, const char *str2)
{
  while (*str1 && *str2 && (*str1++ == *str2++))
    ;


  if (*str1 == '\0' && *str2 == '\0')
    return 0;


  if (*str1 == '\0')
    return -1;
  else return 1;
}

bool strrcmp(const char *start, const char *stop, const char *str)
{
	while(*start == *str)
	{
		if(start >= stop)
			return false;
        
		start++;
		str++;
 
		if(*str == 0)
			return start == stop;
    }
	
	return false;
}

unsigned int strlen(const char *src)
{
  unsigned int i = 0;
  //while (*src++) i++;
  return i;
}
