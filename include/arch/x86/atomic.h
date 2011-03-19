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

#ifndef __ARCH_X86_ATOMIC_H__
#define __ARCH_X86_ATOMIC_H__

static inline uint32 Atomic_TestAndSet( uint32 *pnValue ) {
    uint32 nReturn;
asm volatile( "xchgl %0, %1;" : "=r"( nReturn ), "=m"( *pnValue ) : "0"( 1 ), "m"( *pnValue ) : "memory" );
    return nReturn;
}

static inline void Atomic_Add( uint32 *pnTarget, uint32 nValue ) {
asm volatile( "lock; addl %1,%0;" : "=m" ( *pnTarget ) : "r" ( nValue ), "m" ( *pnTarget ) );
}

static inline void Atomic_Sub( uint32 *pnTarget, uint32 nValue ) {
asm volatile( "lock; subl %1,%0;" : "=m" ( *pnTarget ) : "r" ( nValue ), "m" ( *pnTarget ) );
}

#endif /* __ARCH_X86_ATOMIC_H__ */
