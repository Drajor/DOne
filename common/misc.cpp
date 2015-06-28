#include "misc.h"

void dump_message_column(unsigned char *buffer, unsigned long length, std::string leader, FILE *to)
{
unsigned long i,j;
unsigned long rows,offset=0;
	rows=(length/16)+1;
	for(i=0;i<rows;i++) {
		fprintf(to, "%s%05ld: ",leader.c_str(),i*16);
		for(j=0;j<16;j++) {
			if(j == 8)
				fprintf(to, "- ");
			if (offset+j<length)
				fprintf(to, "%02x ",*(buffer+offset+j));
			else
				fprintf(to, "   ");
		}
		fprintf(to, "| ");
		for(j=0;j<16;j++,offset++) {
			if (offset<length) {
				char c=*(buffer+offset);
				fprintf(to, "%c",isprint(c) ? c : '.');
			}
		}
		fprintf(to, "\n");
	}
}

std::string long2ip(unsigned long ip)
{
char temp[16];
union { unsigned long ip; struct { unsigned char a,b,c,d; } octet;} ipoctet;

	ipoctet.ip=ip;
	sprintf(temp,"%d.%d.%d.%d",ipoctet.octet.a,ipoctet.octet.b,ipoctet.octet.c,ipoctet.octet.d);

	return std::string(temp);
}

void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding)
{
char *ptr=out_buffer;
int i;
char printable[17];
	ptr+=sprintf(ptr,"%0*lu:",padding,offset);
	for(i=0;i<16; i++) {
		if (i==8) {
			strcpy(ptr," -");
			ptr+=2;
		}
		if (i+offset < length) {
			unsigned char c=*(const unsigned char *)(buffer+offset+i);
			ptr+=sprintf(ptr," %02x",c);
			printable[i]=isprint(c) ? c : '.';
		} else {
			ptr+=sprintf(ptr,"   ");
			printable[i]=0;
		}
	}
	sprintf(ptr,"  | %.16s",printable);
}

