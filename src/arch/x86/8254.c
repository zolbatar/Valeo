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

#include <kernel.h>

void PIT_Init ()
{

    outb ( 0x61, inb ( 0x61 ) | 1 );

    // Select counter 0, set to read/load LSB then MSB,
    // mode 2 (Rate generator), Binary 16 bit
    outb ( 0x43, 0x34 );
    outb ( 0x40, PIT_SPEED & 0xff );
    outb ( 0x40, PIT_SPEED >> 8 );

    // Stop counter 1
    outb ( 0x43, 0x74 );
    outb ( 0x42, 0 );
    outb ( 0x42, 0 );

    // Stop counter 2
    outb ( 0x43, 0xB4 );
    outb ( 0x42, 0 );
    outb ( 0x42, 0 );
}

void PIT_StopTimer ()
{

    // Select counter 0, set to read/load LSB then MSB,
    // mode 2 (Rate generator), Binary 16 bit. Stop counter 0
    outb ( 0x43, 0x34 );
    outb ( 0x40, 0 );
    outb ( 0x40, 0 );
}
