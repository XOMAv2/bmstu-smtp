//
// Created by Никита Иксарица on 23.02.2022.
//

#ifndef SMTP_COMMON_HELPERS_H
#define SMTP_COMMON_HELPERS_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

bool string_ends_with(const char *string, const char *suffix);
void ensure_trailing_slash(char *string);

#endif //SMTP_COMMON_HELPERS_H
