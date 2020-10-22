/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */

#include "mini-printf.h"

static unsigned int
mini_strlen(const char *s)
{
    unsigned int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

static unsigned int
mini_itoa(long value, unsigned int radix, unsigned int uppercase, unsigned int unsig,
     char *buffer)
{
    char    *pbuffer = buffer;
    int    negative = 0;
    unsigned int    i, len;

    /* No support for unusual radixes. */
    if (radix > 16)
        return 0;

    if (value < 0 && !unsig) {
        negative = 1;
        value = -value;
    }

    /* This builds the string back to front ... */
    do {
        int digit = value % radix;
        *(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
        value /= radix;
    } while (value > 0);

    if (negative)
        *(pbuffer++) = '-';

    *(pbuffer) = '\0';

    /* ... now we reverse it (could do it recursively but will
     * conserve the stack space) */
    len = (pbuffer - buffer);
    for (i = 0; i < len / 2; i++) {
        char j = buffer[i];
        buffer[i] = buffer[len-i-1];
        buffer[len-i-1] = j;
    }

    return len;
}

static unsigned int
mini_pad(char* ptr, unsigned int len, char pad_char, unsigned int pad_to, char *buffer)
{
    int i;
    int overflow = 0;
    char * pbuffer = buffer;
    if(pad_to == 0) pad_to = len;
    if(len > pad_to) {
        len = pad_to;
        overflow = 1;
    }
    for(i = pad_to - len; i > 0; i --) {
        *(pbuffer++) = pad_char;
    }
    for(i = len; i > 0; i --) {
        *(pbuffer++) = *(ptr++);
    }
    len = pbuffer - buffer;
    if(overflow) {
        for (i = 0; i < 3 && pbuffer > buffer; i ++) {
            *(pbuffer-- - 1) = '*';
        }
    }
    return len;
}

struct mini_buff {
    char *buffer, *pbuffer;
    unsigned int buffer_len;
};

static int
_puts(char *s, unsigned int len, struct mini_buff *b)
{
    unsigned int i;
    /* Copy to buffer */
    for (i = 0; i <= len; i++) {
        if(b->pbuffer - b->buffer + 1 < b->buffer_len) {
            *(b->pbuffer) = (i != len)?s[i]:0;
        }
        if(i != len)
            b->pbuffer ++;
    }
    return len;
}

#ifdef mini_printf_object_t
static int (*mini_handler) (void* data, mini_printf_object_t* obj, int ch, int lhint, char** bf) = 0;
static void (*mini_handler_freeor)(void* data, void*) = 0;
static void * mini_handler_data = 0;

void mini_printf_set_handler(
    void* data,
    int (*handler)(void* data, mini_printf_object_t* obj, int ch, int len_hint, char** buf),
    void (*freeor)(void* data, void* buf))
{
    mini_handler = handler;
    mini_handler_freeor = freeor;
    mini_handler_data = data;
}
#endif

int
mini_vsnprintf(char *buffer, unsigned int buffer_len, const char *fmt, va_list va)
{
    struct mini_buff b;
    char bf[24];
    char bf2[24];
    char ch;
    mini_printf_object_t obj;
    b.buffer = buffer;
    b.pbuffer = buffer;
    b.buffer_len = buffer_len;

    while ((ch=*(fmt++))) {
        if (ch!='%')
            _puts(&ch, 1, &b);
        else {
            char pad_char = ' ';
            unsigned int pad_to = 0;
            char l = 0;
            char *ptr;
            unsigned int len;

            ch=*(fmt++);

            /* Zero padding requested */
            if (ch == '0') pad_char = '0';
            while (ch >= '0' && ch <= '9') {
                pad_to = pad_to * 10 + (ch - '0');
                ch=*(fmt++);
            }
            if(pad_to > sizeof(bf)) {
                pad_to = sizeof(bf);
            }
            if (ch == 'l') {
                l = 1;
                ch=*(fmt++);
            }

            switch (ch) {
                case 0:
                    goto end;
                case 'u':
                case 'd':
                    if(l) {
                        len = mini_itoa(va_arg(va, unsigned long), 10, 0, (ch=='u'), bf2);
                    } else {
                        if(ch == 'u') {
                            len = mini_itoa((unsigned long) va_arg(va, unsigned int), 10, 0, 1, bf2);
                        } else {
                            len = mini_itoa((long) va_arg(va, int), 10, 0, 0, bf2);
                        }
                    }
                    len = mini_pad(bf2, len, pad_char, pad_to, bf);
                    _puts(bf, len, &b);
                    break;

                case 'x':
                case 'X':
                    if(l) {
                        len = mini_itoa(va_arg(va, unsigned long), 16, (ch=='X'), 1, bf2);
                    } else {
                        len = mini_itoa((unsigned long) va_arg(va, unsigned int), 16, (ch=='X'), 1, bf2);
                    }
                    len = mini_pad(bf2, len, pad_char, pad_to, bf);
                    _puts(bf, len, &b);
                    break;

                case 'c' :
                    ch = (char)(va_arg(va, int));
                    len = mini_pad(&ch, 1, pad_char, pad_to, bf);
                    _puts(bf, len, &b);
                    break;

                case 's' :
                    ptr = va_arg(va, char*);
                    len = mini_strlen(ptr);
                    if (pad_to > 0) {
                        len = mini_pad(ptr, len, pad_char, pad_to, bf);
                        _puts(bf, len, &b);
                    } else {
                        _puts(ptr, mini_strlen(ptr), &b);
                    }
                    break;
                #ifdef mini_printf_object_t
                case 'o' :
                case 'O' :
                    obj = va_arg(va, mini_printf_object_t);
                    len = mini_handler(mini_handler_data, &obj, ch, pad_to, &ptr);
                    if (pad_to > 0) {
                        len = mini_pad(ptr, len, pad_char, pad_to, bf);
                        _puts(bf, len, &b);
                    } else {
                        _puts(ptr, len, &b);
                    }
                    mini_handler_freeor(mini_handler_data, ptr);
                    break;
                #endif
                default:
                    _puts(&ch, 1, &b);
                    break;
            }
        }
    }
end:
    return b.pbuffer - b.buffer;
}


int
mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    ret = mini_vsnprintf(buffer, buffer_len, fmt, va);
    va_end(va);

    return ret;
}

