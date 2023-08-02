/*
 *  Utilities.c
 *  Brogue
 *
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"

// String Functions
boolean endswith(const char *str, const char *ending)
{
    int str_len = strlen(str), ending_len = strlen(ending);
    if (str_len < ending_len) return false;
    return strcmp(str + str_len - ending_len, ending) == 0 ? true : false;
}

void append(char *str, char *ending, int bufsize) {
    int str_len = strlen(str), ending_len = strlen(ending);
    if (str_len + ending_len + 1 > bufsize) return;
    strcpy(str + str_len, ending);
}
