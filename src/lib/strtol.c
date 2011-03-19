/***************************************************************************
*   Copyright (C) 2004 by Daryl Dudey                                     *
*   daryl_dudey_uk@hotmail.com                                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include <strtol.h>

long int strtol( const char *nptr, char **endptr, int base ) {

  int neg = 0;
  unsigned long int v;

  while ( isspace( *nptr ) )
    nptr++;

  if ( *nptr == '-' ) {
    neg = -1;
    ++nptr;
  }
  v = strtoul( nptr, endptr, base );
  if ( v >= ABS_LONG_MIN ) {
    if ( v == ABS_LONG_MIN && neg ) {
      return v;
    }
    return 0;
  }
  return ( neg ? -v : v );
}

unsigned long int strtoul( const char *nptr, char **endptr, int base ) {

  unsigned long int v = 0;

  while ( isspace( *nptr ) )
    ++nptr;
  if ( *nptr == '+' )
    ++nptr;
  if ( base == 16 && nptr[ 0 ] == '0' )
    goto skip0x;
  if ( !base ) {
    if ( *nptr == '0' ) {
      base = 8;
    skip0x:
      if ( nptr[ 1 ] == 'x' || nptr[ 1 ] == 'X' ) {
        nptr += 2;
        base = 16;
      }
    } else
      base = 10;
  }
  while ( *nptr ) {
    register unsigned char c = *nptr;
    c = ( c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c <= '9' ? c - '0' : 0xff );
    if ( c >= base )
      break;
    {
      register unsigned long int w = v * base;
      if ( w < v ) {
        return 0;
      }
      v = w + c;
    }
    ++nptr;
  }
  if ( endptr )
    * endptr = ( char * ) nptr;
  return v;
}
