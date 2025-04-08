#ifndef SAFE_H
#define SAFE_H
char *safe_strcat(char *lhs, const char *rhs);
void *safe_alloc(long size);
char *safe_realloc_str(char *ptr, long size);
char *safe_strcpy(char *dptr, char *sptr);
#endif
