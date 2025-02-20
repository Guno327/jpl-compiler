#ifndef SAFE_H
#define SAFE_H
char *safe_strcat(char *lhs, const char *rhs);
void *safe_alloc(int size);
char *safe_realloc_str(char *ptr, int size);
#endif
