#ifndef _MISC_H
#define _MISC_H

#include <stdio.h>
#include <string>

void dump_message_column(unsigned char *buffer, unsigned long length, std::string leader="", FILE *to = stdout);
std::string long2ip(unsigned long ip);
void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding=4);

#endif

