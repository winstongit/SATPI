/* utils.c

   Copyright (C) 2014 Marc Postema

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include <linux/dvb/frontend.h>

#include "utils.h"
#include "applog.h"

/*
 *
 */
char *get_line_from(const char *buf, size_t *ptr_index, const char *line_delim) {
	char *line = NULL;
	size_t buf_size = strlen(buf);
	if (buf && buf_size > *ptr_index) {
		const char *begin = buf + *ptr_index;
		size_t size = 0;
		const char *end = strstr(begin, line_delim);
		if (end != NULL) {
			size = end - begin;
			*ptr_index += size + strlen(line_delim);
		} else if (begin) {
			size = strlen(begin);
			*ptr_index += size;
		}
		if (size) {
			line = malloc(size + 1);
			if (line == NULL) {
				return NULL;
			}
			memcpy(line, begin, size);
			line[size] = 0;
		} else {
			line = malloc(17);
			if (line == NULL) {
				return NULL;
			}
			sprintf(line, "--- LINE END ---");
		}
	}
	return line;
}

/*
 *
 */
void get_method_from(const char *buf, char *method) {
	char *line = NULL;
	size_t ptr_index = 0;
	size_t i;
	// request line should be in the first line (method)
	line = get_line_from(buf, &ptr_index, "\r\n");
	if (line) {
		i = 0;
		// remove any leading whitespace
		while (line[i] == ' ') ++i;

		// copy method (upper case)
		while (line[i] != ' ') {
			*method = toupper(line[i]);
			++method;
			++i;
		}
		*method = 0;
		FREE_PTR(line);
	}
}

/*
 *
 */
char *get_header_field_from(const char *buf, const char *header_field) {
	char *line = NULL;
	size_t ptr_index = 0;
	size_t i, j;
	do {
		line = get_line_from(buf, &ptr_index, "\r\n");
		if (line) {
			i = 0;
			j = 0;
			// remove any leading whitespace
			while (line[i] == ' ') ++i;
			
			int found = 1;
			while (line[i] != ':' && line[i] != ' ') {
				if (toupper(line[i]) != toupper(header_field[j])) {
					found = 0;
					break;
				}
				++i;
				++j;
			}
			if (found) {
				break;
			}
			free(line); // Do not use FREE_PTR (it will do line = NULL)
		}
	} while (line != NULL);
	return line;
}

/*
 *
 */
char *get_header_field_parameter_from(const char *buf, const char *header_field) {
	char *line = get_header_field_from(buf, header_field);
	if (line) {
		char *val;
		strtok_r(line, ":", &val);
		// remove any leading whitespace
		while (*val == ' ') ++val;
		// copy
		size_t param_size = strlen(val);
		char *param = malloc(param_size);
		memcpy(param, val, param_size);
		FREE_PTR(line);
		return param;
	}
	return NULL;
}


/*
 *
 */
char *get_content_type_from(const char *msg) {
	// find 'content-length' first to see if there is something
	char *line = get_header_field_from(msg, "Content-Length");
	if (line) {
		char *val;
		strtok_r(line, ":", &val);
		size_t size = atoi(val);
		FREE_PTR(line);
		if (size > 0) {
			const char *begin = strstr(msg, "\r\n\r\n");
			if (begin) {
				char *ptr = malloc(size + 1);
				if (ptr) {
					memcpy(ptr, (begin + 4), size);
					*(ptr + size) = 0;
					return ptr;
				}
			}
		}
	}
	return NULL;
}

/*
 *
 */
char *make_xml_string(char *str) {
	char *newStr = NULL;
	char *begin = str;
	char *end = str;
	do {
		if (*end == '&') {
			*end = 0;
			addString(&newStr, "%s&amp;", begin);
			*end = '&';
			begin = end + 1;
		} else if (*end == '"') {
			*end = 0;
			addString(&newStr, "%s&quot;", begin);
			*end = '"';
			begin = end + 1;
		} else if (*end == '>') {
			*end = 0;
			addString(&newStr, "%s&gt;", begin);
			*end = '>';
			begin = end + 1;
		} else if (*end == '<') {
			*end = 0;
			addString(&newStr, "%s&lt;", begin);
			*end = '<';
			begin = end + 1;
		}
		++end;
	} while (*end != '\0');
	addString(&newStr, "%s", begin);
	return newStr;
}

/*
 *
 */
size_t addString(char **str, const char *fmt, ...) {
    char txt[1024];
    va_list arglist;
    va_start(arglist, fmt);
    const size_t cnt = vsnprintf(txt, sizeof(txt)-1, fmt, arglist);
    va_end(arglist);
	txt[cnt] = 0;
	if (*str == NULL) {
		*str = malloc(strlen(txt) + 1);
		if (*str == NULL) {
			PERROR("addString malloc");
			return 0;
		}
		*str[0] = 0;
	} else {
		const size_t len = strlen(txt) + strlen(*str);
		char *ptr = *str;
		*str = realloc(*str, len + 1);
		if (*str == NULL) {
			PERROR("addString realloc");
			*str = ptr;
			return 0;
		}
	}
	strcat(*str, txt);
	return strlen(*str);
}

/*
 *
 */
long getmsec() {
  struct timeval tv;
  gettimeofday(&tv,(struct timezone*) NULL);
  return(tv.tv_sec%1000000)*1000 + tv.tv_usec/1000;
}
