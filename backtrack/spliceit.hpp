#ifndef SPLICEIT_H
#define SPLICEIT_H

#include "exceptions.hpp"

extern const char *ME;
extern const char *INPUT_SEPARATOR;

void enlarge_fields(char ***fields, int *cur_nfields);
int str_split(char *line, const char *sep, char ***fields);
void chop(const char *input, char **output);

#endif
