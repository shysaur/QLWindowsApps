/* strbuf.h - The string buffer data-structure.
 *
 * Copyright (C) 2004 Oskar Liljeblad
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* TODO: add datatype (many functions): strbuf_substring / substrbuf */

#ifndef STRBUF_H
#define STRBUF_H

#include <stdint.h>	/* Gnulib/C99 */
#include <stdarg.h>	/* Gnulib/C89 */

typedef struct _StrBuf StrBuf;

struct _StrBuf {
    char *buf;
    uint32_t len;
    uint32_t capacity;
};

void strbuf_free(StrBuf *sb);
#define strbuf_free_to_string(sb)			strbuf_free_to_substring(sb,0,-1)
char *strbuf_free_to_substring(StrBuf *sb, int32_t sp, int32_t ep);

StrBuf *strbuf_new(void);
StrBuf *strbuf_new_with_capacity(uint32_t capacity);

#define strbuf_new_from_char(chr)			strbuf_new_from_char_n(1,chr)
#define strbuf_new_from_string(str) 	    	    	strbuf_new_from_substring(str,0,-1)
#define strbuf_new_from_data(mem,len)			strbuf_new_from_data_n(str,1,mem,len)
#define strbuf_new_from_strbuf(strbuf)			strbuf_new_from_strbuf_n(1,strbuf)
#define strbuf_new_from_substring(str,ssp,sep)		strbuf_new_from_substring_n(1,str,ssp,sep)
#define strbuf_newf(fmt...)				strbuf_newf_n(1,fmt)
#define strbuf_vnewf(fmt,ap)				strbuf_vnewf_n(1,fmt,ap)
StrBuf *strbuf_new_from_char_n(uint32_t times, char ch);
#define strbuf_new_from_string_n(n,str)			strbuf_new_from_substring_n(n,str,0,-1)
StrBuf *strbuf_new_from_substring_n(uint32_t times, const char *str, int32_t sp, int32_t ep);
StrBuf *strbuf_new_from_data_n(uint32_t times, const void *mem, uint32_t len);
StrBuf *strbuf_newf_n(uint32_t times, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
StrBuf *strbuf_vnewf_n(uint32_t times, const char *fmt, va_list ap) __attribute__ ((format (printf, 2, 0)));

#define strbuf_append_char(sb,chr)   	    	        strbuf_append_char_n(sb,1,chr)
#define strbuf_append(sb,str) 	    	    	        strbuf_append_n(sb,1,str)
#define strbuf_append_data(sb,mem,len)   	    	strbuf_append_data_n(sb,1,mem,len)
#define strbuf_append_strbuf(sb,strbuf)			strbuf_append_strbuf_n(sb,1,strbuf)
#define strbuf_append_substring(sb,str,ssp,sep)	        strbuf_append_substring_n(sb,1,str,ssp,sep)
#define strbuf_appendf(sb,fmt...)   	    	        strbuf_appendf_n(sb,1,fmt)
#define strbuf_vappendf(sb,fmt,ap)  	    	        strbuf_vappendf_n(sb,1,fmt,ap)
#define strbuf_append_char_n(sb,n,chr)                  strbuf_replace_char_n(sb,-1,-1,n,chr)
#define strbuf_append_n(sb,n,str)                       strbuf_replace_n(sb,-1,-1,n,str)
#define strbuf_append_data_n(sb,n,mem,len)   	    	strbuf_replace_data_n(sb,-1,-1,n,mem,len)
#define strbuf_append_strbuf_n(sb,n,strbuf)		strbuf_replace_strbuf_n(sb,-1,-1,n,strbuf)
#define strbuf_append_substring_n(sb,n,str,ssp,sep)     strbuf_replace_substring_n(sb,-1,-1,n,str,ssp,sep)
#define strbuf_appendf_n(sb,n,fmt...)                   strbuf_replacef_n(sb,-1,-1,n,fmt)
#define strbuf_vappendf_n(sb,n,fmt,ap)                  strbuf_vreplacef_n(sb,-1,-1,n,fmt,ap)

#define strbuf_prepend_char(sb,chr)   	    	        strbuf_prepend_char_n(sb,1,chr)
#define strbuf_prepend(sb,str) 	    	    	        strbuf_prepend_n(sb,1,str)
#define strbuf_prepend_data(sb,mem,len) 	        strbuf_prepend_data_n(sb,1,mem,len)
#define strbuf_prepend_strbuf(sb,strbuf)		strbuf_prepend_strbuf_n(sb,1,strbuf)
#define strbuf_prepend_substring(sb,str,ssp,sep)        strbuf_prepend_substring_n(sb,1,str,ssp,sep)
#define strbuf_prependf(sb,fmt...)   	    	        strbuf_prependf_n(sb,1,fmt)
#define strbuf_vprependf(sb,fmt,ap)  	    	        strbuf_vprependf_n(sb,1,fmt,ap)
#define strbuf_prepend_char_n(sb,n,chr)                 strbuf_replace_char_n(sb,0,0,n,chr)
#define strbuf_prepend_n(sb,n,str)                      strbuf_replace_n(sb,0,0,n,str)
#define strbuf_prepend_data_n(sb,n,mem,len) 	        strbuf_replace_data_n(sb,0,0,n,mem,len)
#define strbuf_prepend_strbuf_n(sb,n,strbuf)		strbuf_replace_strbuf_n(sb,0,0,n,strbuf)
#define strbuf_prepend_substring_n(sb,n,str,ssp,sep)    strbuf_replace_substring_n(sb,0,0,n,str,ssp,sep)
#define strbuf_prependf_n(sb,n,fmt...)                  strbuf_replacef_n(sb,0,0,n,fmt)
#define strbuf_vprependf_n(sb,n,fmt,ap)                 strbuf_vreplacef_n(sb,0,0,n,fmt,ap)

#define strbuf_insert_char(sb,sp,chr)   	        strbuf_insert_char_n(sb,sp,1,chr)
#define strbuf_insert(sb,sp,str) 	    	        strbuf_insert_n(sb,sp,1,str)
#define strbuf_insert_data(sb,sp,mem,len) 	        strbuf_insert_data_n(sb,sp,1,mem,len)
#define strbuf_insert_strbuf(sb,sp,strbuf)		strbuf_insert_strbuf_n(sb,sp,1,strbuf)
#define strbuf_insert_substring(sb,sp,str,ssp,sep)      strbuf_insert_substring_n(sb,sp,1,str,ssp,sep)
#define strbuf_insertf(sb,sp,fmt...)   	    	        strbuf_insertf_n(sb,sp,1,fmt)
#define strbuf_vinsertf(sb,sp,fmt,ap)  	    	        strbuf_vinsertf_n(sb,sp,1,fmt,ap)
#define strbuf_insert_char_n(sb,sp,n,chr)               strbuf_replace_char_n(sb,sp,sp,n,chr)
#define strbuf_insert_n(sb,sp,n,str)                    strbuf_replace_n(sb,sp,sp,n,str)
#define strbuf_insert_data_n(sb,sp,n,mem,len) 	        strbuf_replace_data_n(sb,sp,sp,n,mem,len)
#define strbuf_insert_strbuf_n(sb,sp,n,strbuf)		strbuf_replace_strbuf_n(sb,sp,sp,n,strbuf)
#define strbuf_insert_substring_n(sb,sp,n,str,ssp,sep)  strbuf_replace_substring_n(sb,sp,sp,n,str,ssp,sep)
#define strbuf_insertf_n(sb,sp,n,fmt...)                strbuf_replacef_n(sb,sp,sp,n,fmt)
#define strbuf_vinsertf_n(sb,sp,n,fmt,ap)               strbuf_vreplacef_n(sb,sp,sp,n,fmt,ap)

#define strbuf_set_char(sb,chr) 	    	        strbuf_set_char_n(sb,1,chr)
#define strbuf_set(sb,str)  	    	    	        strbuf_set_n(sb,1,str)
#define strbuf_set_data(sb,mem,len)  	            	strbuf_set_data_n(sb,1,mem,len)
#define strbuf_set_strbuf(sb,strbuf)			strbuf_set_strbuf(sb,1,strbuf)
#define strbuf_set_substring(sb,str,ssp,sep)  	        strbuf_set_substring_n(sb,1,str,ssp,sep)
#define strbuf_setf(sb,fmt...)  	    	        strbuf_setf_n(sb,1,fmt)
#define strbuf_vsetf(sb,fmt,ap)  	    	        strbuf_vsetf_n(sb,1,fmt,ap)
#define strbuf_set_char_n(sb,n,chr)                     strbuf_replace_char_n(sb,0,-1,n,chr)
#define strbuf_set_n(sb,n,str)                          strbuf_replace_n(sb,0,-1,n,str)
#define strbuf_set_data_n(sb,n,mem,len)  	        strbuf_replace_data_n(sb,0,-1,n,mem,len)
#define strbuf_set_strbuf_n(sb,n,strbuf)		strbuf_replace_strbuf_n(sb,0,-1,n,strbuf)
#define strbuf_set_substring_n(sb,n,str,ssp,sep)        strbuf_replace_substring_n(sb,0,-1,n,str,ssp,sep)
#define strbuf_setf_n(sb,n,fmt...)                      strbuf_replacef_n(sb,0,-1,n,fmt)
#define strbuf_vsetf_n(sb,n,fmt,ap)                     strbuf_vreplacef_n(sb,0,-1,n,fmt,ap)

#define strbuf_replace_char(sb,sp,ep,ch)                strbuf_replace_char_n(sb,sp,ep,1,ch)
#define strbuf_replace(sb,sp,ep,str)	    	        strbuf_replace_n(sb,sp,ep,1,str)
#define strbuf_replace_data(sb,sp,ep,mem,len)	    	strbuf_replace_data_n(sb,sp,ep,1,mem,len)
void strbuf_replace_strbuf(StrBuf *sb, int32_t sp, int32_t ep, StrBuf *strbuf); /* or strbuf_replace_data_n(sb,sp,ep,1,strbuf_buffer(strbuf),strbuf_length(strbuf))*/
#define strbuf_replace_substring(sb,sp,ep,str,ssp,sep)  strbuf_replace_substring_n(sb,sp,ep,1,str,ssp,sep)
#define strbuf_replacef(sb,sp,ep,fmt...)                strbuf_replacef_n(sb,sp,ep,1,fmt)
#define strbuf_vreplacef(sb,sp,ep,fmt,ap)               strbuf_vreplacef_n(sb,sp,ep,1,fmt,ap)
void strbuf_replace_char_n(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times, char ch);
#define strbuf_replace_n(sb,sp,ep,n,str)	    	strbuf_replace_substring_n(sb,sp,ep,n,str,0,-1)
void strbuf_replace_substring_n(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times, const char *str, int32_t ssp, int32_t sep);
void strbuf_replace_data_n(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times, const void *mem, uint32_t len);
int strbuf_replacef_n(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times, const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));
int strbuf_vreplacef_n(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times, const char *fmt, va_list ap) __attribute__ ((format (printf, 5, 0)));

char strbuf_set_char_at(StrBuf *sb, int32_t sp, char chr); 	/* or strbuf_replace_char(sb,sp,strbuf_next_pos(sb,sp),chr) */
char strbuf_delete_char(StrBuf *sb, int32_t sp); 		/* or strbuf_replace(sb,sp,strbuf_next_pos(sb,sp),NULL) */
#define strbuf_delete(sb,sp,ep)   	    	        strbuf_replace(sb,sp,ep,NULL)
#define strbuf_clear(sb)   	    	    	        strbuf_replace(sb,0,-1,NULL)

void strbuf_reverse_substring(StrBuf *sb, int32_t sp, int32_t ep);
#define strbuf_reverse(sb)  	    	    	        strbuf_reverse_substring(sb,0,-1)

void strbuf_repeat_substring(StrBuf *sb, int32_t sp, int32_t ep, uint32_t times);
#define strbuf_repeat(sb,n)  	    	    	        strbuf_repeat_substring(sb,0,-1,n)

uint32_t strbuf_length(StrBuf *sb);
uint32_t strbuf_capacity(StrBuf *sb);
char *strbuf_buffer(StrBuf *sb);
#define strbuf_is_empty(sb)				(strbuf_length(sb) == 0)

void strbuf_set_length(StrBuf *sb, uint32_t new_length);
void strbuf_ensure_capacity(StrBuf *sb, uint32_t minimum_capacity); /* minimum_capacity should account for null-byte */

char *strbuf_substring(StrBuf *sb, int32_t sp, int32_t ep);
#define strbuf_to_string(sb)	    	    	        strbuf_substring(sb,0,-1)
char strbuf_char_at(StrBuf *sb, int32_t index);

#endif
