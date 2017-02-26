#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exceptions.hpp"

const char * ME = "spliceit";
const char * INPUT_SEPARATOR = " ";

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



int main(int argc, const char ** argv)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int nfields;
    char** fields;

    // Parse parameters.
    if(argc < 2){
      printf("error: not enough arguments\n");
      printf("Usage: ./%s input_file output_file\n", ME);
      printf("\n");
      printf("Process a CSV file.  It can currenlty discard some columns and add\n");
      printf("string lengths of some columns as new columns.\n");
      printf("The input is assumed to be space separated and output will be\n");
      printf("semicolong separated.\n");
      printf("\n");
      printf("You can configure the parameters in the source code and recompile.\n");
      exit(EXIT_FAILURE);
    }
    const char* input_filename = argv[1];

    // Open files.
    FILE* infile = fopen(input_filename, "r");
    if (infile == NULL){
      printf("failed to open input file\n");
      exit(EXIT_FAILURE);
    }

    // First line is special
    if((read = getline(&line, &len, infile)) == -1){
      printf("fatal");
    }
    nfields = str_split(line, INPUT_SEPARATOR, &fields);
    printf("starting city = %s\n", fields[0]);
    int i;
    for(i = 0; i < nfields; i++)
      free(fields[i]);
    free(fields);

    while ((read = getline(&line, &len, infile)) != -1) {
      nfields = str_split(line, INPUT_SEPARATOR, &fields);

      // Include selected fields.
      printf("%s\n", fields[0]);
      exit(1);
      int i;
      for(i = 0; i < nfields; i++)
	free(fields[i]);
      free(fields);
    }

    fclose(infile);
    free(line);
    exit(EXIT_SUCCESS);
}
