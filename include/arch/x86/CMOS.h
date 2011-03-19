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

#ifndef __ARCH_X86_CMOS_H__
#define __ARCH_X86_CMOS_H__

static inline uchar CMOS_Read( uchar nIndex ) {
    // Read in CMOS data register
    uchar nSaved = inb ( 0x70 );

    // Mask 5 lowest bits
    nSaved &= 0xE0;

    // Or the index we want
    nSaved |= nIndex;

    // Send our request
    outb( nSaved, 0x70 );

    // And return the result
    return inb ( 0x71 );
}

#endif /* __ARCH_X86_CMOS_H__ */
