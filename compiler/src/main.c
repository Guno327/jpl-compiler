#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "lexer.h"

int main(int argc, char** argv){
  char* input = "this!isnot:wq";
  Token* t = malloc(sizeof(Token));
  int ret = 0;
  ret = lex_wrd(input, 0, t);
  printf("%s\n", print_token(t));
  exit(EXIT_SUCCESS);

// if (argc < 2){
//   fprintf(stderr, "No file given\n");
//   exit(EXIT_FAILURE);
// };

// // Open File
// FILE *src_file;
// src_file = fopen(argv[1], "r");
// if (src_file == NULL){
//   fprintf(stderr, "Could not open file: %s\n", argv[2]);
//   exit(EXIT_FAILURE);
// };

// // Read File
// char *src = malloc(BUFSIZ);
// char *buf = malloc(BUFSIZ);

// int read = 0;

// do {
//   fgets(buf, BUFSIZ, src_file);
//   read = strlen(buf);
//   src = realloc(src, strlen(src) + read + 1);

//   if (src == NULL){
//     fprintf(stderr, "Malloc failed\n");
//     exit(EXIT_FAILURE);
//   }

//   strncat(src, buf, read);
//   memset(buf, 0, BUFSIZ);
// } while (read!=0);

// Vector* tokens = NULL;
// tokens = lex(src);
}
