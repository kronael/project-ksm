#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exceptions.hpp"

const char *ME = "spliceit";
const char *INPUT_SEPARATOR = " ";

void enlarge_fields(char*** fields, int* cur_nfields)
{
    *cur_nfields *= 2;
    *fields = (char**)realloc(*fields, *cur_nfields * sizeof(char*));
    if(*fields == NULL){
      AllocError error("can't realoc fields for reading");
      throw error;
    }
}


int str_split(char* line, const char* sep, char*** fields)
{
    const char* tok;
    int cur_nfields = 3;
    *fields = NULL;
    enlarge_fields(fields, &cur_nfields);

    int i = 0;
    for(tok = strsep(&line, sep);
	tok && *tok;
	 tok = strsep(&line, sep)){
      size_t tok_len = strlen(tok);
      (*fields)[i] = (char*)malloc(sizeof(char*) * tok_len);
      strncpy((*fields)[i], tok, tok_len + 1);
      i++;
    }
    return i;
}


// Beware that the output must be freed.
void chop(const char *input, char **output)
{
  int in_size = strlen(input);
  *output = (char*)malloc(in_size * sizeof(char));
  strncpy(*output, input, in_size - 1);
  // Assure NULL termination.
  (*output)[in_size - 1] = '\0';
}
