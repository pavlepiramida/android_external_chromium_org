/* udis86 - libudis86/syn.h
 *
 * Copyright (c) 2002-2009
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation 
 *       and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef UD_SYN_H
#define UD_SYN_H

#include "types.h"
#ifndef __UD_STANDALONE__
# include <stdarg.h>
#endif /* __UD_STANDALONE__ */

extern const char* ud_reg_tab[];

uint64_t ud_syn_rel_target(struct ud*, struct ud_operand*);

#ifdef __GNUC__
int ud_asmprintf(struct ud *u, char *fmt, ...) 
    __attribute__ ((format (printf, 2, 3)));
#else
int ud_asmprintf(struct ud *u, char *fmt, ...);
#endif

void ud_syn_print_addr(struct ud *u, uint64_t addr);
void ud_syn_print_imm(struct ud* u, const struct ud_operand *op);
void ud_syn_print_mem_disp(struct ud* u, const struct ud_operand *, int sign);

#endif /* UD_SYN_H */

/*
vim: set ts=2 sw=2 expandtab
*/
