#ifndef __array_H                                      // -*- c++ -*-
#define __array_H
/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: array.h,v 1.2 2007/04/10 01:26:16 steve Exp $"
#endif

#include "vvp_net.h"
#include "vpi_user.h"

typedef struct __vpiArray* vvp_array_t;

/*
 * This function tries to find the array (by label) in the global
 * table of all the arrays in the design.
 */
extern vvp_array_t array_find(char*label);
extern vpiHandle array_index_iterate(int code, vpiHandle ref);

extern void array_word_change(vvp_array_t array, unsigned long addr);

extern void array_attach_word(vvp_array_t array, unsigned long addr, vpiHandle word);

extern void array_set_word(vvp_array_t arr,
			    unsigned idx,
			    unsigned off,
			    vvp_vector4_t val);

extern vvp_vector4_t array_get_word(vvp_array_t array, unsigned adddress);

extern void compile_variablew(char*label, vvp_array_t array,
			      unsigned long array_addr,
			     int msb, int lsb, char signed_flag);
extern void compile_varw_real(char*label, vvp_array_t array,
			      unsigned long array_addr,
			      int msb, int lsb);
#endif