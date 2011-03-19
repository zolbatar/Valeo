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

#ifndef __ARCH_X86_PIT_H__
#define __ARCH_X86_PIT_H__

/* TICKSPERSEC is a dividor because the PIT ticks at 1,193,181Hz */
#define PIT_TICKSPERSEC 0x1234DD

/* We want the timer to tick at 1000Hz, to give us a nice responsive system */
#define	PIT_FREQ 1000

/* Calculate value to send to PIT */
#define PIT_SPEED (PIT_TICKSPERSEC / PIT_FREQ)

void PIT_Init( void );
void PIT_StopTimer( void );

#endif /* __ARCH_X86_PIT_H__ */
