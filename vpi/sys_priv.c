/*
 * Copyright (c) 2003-2008 Stephen Williams (steve@icarus.com)
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

#include <assert.h>
#include "sys_priv.h"

PLI_UINT64 timerec_to_time64(const struct t_vpi_time*time)
{
      PLI_UINT64 tmp;

      tmp = time->high;
      tmp <<= 32;
      tmp |= (PLI_UINT64) time->low;

      return tmp;
}

/*
 * This routine returns 1 if the argument is a constant value,
 * otherwise it returns 0.
 */
unsigned is_constant_obj(vpiHandle obj)
{
      assert(obj);
      unsigned rtn = 0;

      switch(vpi_get(vpiType, obj)) {
	case vpiConstant:
	case vpiParameter:
	    rtn = 1;
	    break;
      }

      return rtn;
}

/*
 * This routine returns 1 if the argument supports has a numeric value,
 * otherwise it returns 0.
 */
unsigned is_numeric_obj(vpiHandle obj)
{
    assert(obj);
    unsigned rtn = 0;

    switch(vpi_get(vpiType, obj)) {
      case vpiConstant:
      case vpiParameter:
	  /* These cannot be a string constant. */
	if (vpi_get(vpiConstType, obj) != vpiStringConst) rtn = 1;
	break;

	/* These can have a valid numeric value. */
      case vpiIntegerVar:
      case vpiMemoryWord:
      case vpiNet:
      case vpiPartSelect:
      case vpiRealVar:
      case vpiReg:
      case vpiTimeVar:
	rtn = 1;;
	break;
    }

    return rtn;
}


/*
 * This routine returns 1 if the argument supports a valid string value,
 * otherwise it returns 0.
 */
unsigned is_string_obj(vpiHandle obj)
{
    assert(obj);
    unsigned rtn = 0;

    switch(vpi_get(vpiType, obj)) {
      case vpiConstant:
      case vpiParameter: {
	  /* These must be a string or binary constant. */
	PLI_INT32 ctype = vpi_get(vpiConstType, obj);
	if (ctype == vpiStringConst || ctype == vpiBinaryConst) rtn = 1;
	break;
      }

	/* These can have a valid string value. */
      case vpiIntegerVar:
      case vpiMemoryWord:
      case vpiNet:
      case vpiPartSelect:
      case vpiReg:
      case vpiTimeVar:
	rtn = 1;;
	break;
    }

    return rtn;
}


/*
 * Find the enclosing module.
 */
vpiHandle sys_func_module(vpiHandle obj)
{
      assert(obj);

      while (vpi_get(vpiType, obj) != vpiModule) {
	    obj = vpi_handle(vpiScope, obj);
	    assert(obj);
      }

      return obj;
}