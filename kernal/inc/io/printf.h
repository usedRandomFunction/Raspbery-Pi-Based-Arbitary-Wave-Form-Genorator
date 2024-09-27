/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

// Moddifyed to include sprintf_s and reguler printf() (useing putchar())

#ifndef PRINTF_H
#define PRINTF_H

#include <stddef.h>
#include <stdint.h>

unsigned int printf(const char* fmt, ...);
unsigned int vprintf(const char* fmt, __builtin_va_list args);

unsigned int sprintf(char *dst, const char* fmt, ...);
unsigned int vsprintf(char *dst, const char* fmt, __builtin_va_list args);

unsigned int sprintf_s(char *dst, size_t size_of_Buffer, const char* fmt, ...);
unsigned int vsprintf_s(char *dst, size_t size_of_Buffer, const char* fmt, __builtin_va_list args);

#endif