/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_EASE_H
#define INCLUDED_LIB3DS_EASE_H
/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $Id: Lib3DS_ease.h,v 1.6 2007/06/14 09:59:10 jeh Exp $
 */

#ifndef INCLUDED_LIB3DS_TYPES_H
#include <Lib3DS_types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern LIB3DSAPI Lib3dsFloat lib3ds_ease(Lib3dsFloat fp, Lib3dsFloat fc, 
  Lib3dsFloat fn, Lib3dsFloat ease_from, Lib3dsFloat ease_to);

#ifdef __cplusplus
}
#endif
#endif

