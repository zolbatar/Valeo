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

#ifndef _LIBC_SYS_IO_H
#define _LIBC_SYS_IO_H

static inline unsigned char inb ( unsigned short int port ) {
  unsigned char _v;
__asm__ __volatile__ ( "inb %w1,%0": "=a" ( _v ) : "Nd" ( port ) );
  return _v;
}

static inline unsigned short inw ( unsigned short int port ) {
  unsigned short _v;
__asm__ __volatile__ ( "inw %w1,%0": "=a" ( _v ) : "Nd" ( port ) );
  return _v;
}

static inline unsigned int inl ( unsigned short int port ) {
  unsigned int _v;
__asm__ __volatile__ ( "inl %w1,%0": "=a" ( _v ) : "Nd" ( port ) );
  return _v;
}

static inline void outb ( unsigned short int port, unsigned char value ) {
__asm__ __volatile__ ( "outb %b0,%w1": : "a" ( value ), "Nd" ( port ) );
}

static inline void outw ( unsigned short int port, unsigned short value ) {
__asm__ __volatile__ ( "outw %w0,%w1": : "a" ( value ), "Nd" ( port ) );
}

static inline void outl ( unsigned short int port, unsigned int value ) {
__asm__ __volatile__ ( "outl %0,%w1": : "a" ( value ), "Nd" ( port ) );
}

#endif /* __LIB/INTERRUPT_H__ */

