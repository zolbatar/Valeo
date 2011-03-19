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

#ifndef __ARCH_X86_SPINLOCK_H__
#define __ARCH_X86_SPINLOCK_H__

#include <arch/x86/atomic.h>

typedef struct {
    volatile uint32 m_nLock;
}
sSpinlock_t;

static inline void Spinlock_Init( sSpinlock_t *psLock ) {
    psLock->m_nLock = 0;
}

static inline void Spinlock_Lock( sSpinlock_t *psLock ) {

    /* Spin until we get lock */
    while ( Atomic_TestAndSet( ( uint32 * ) & psLock->m_nLock ) )
        ;
}

static inline void Spinlock_Unlock( sSpinlock_t *psLock ) {

    /* Unlock spinlock */
    psLock->m_nLock = 0;
}

static inline bool Spinlock_TryLock( sSpinlock_t *psLock ) {
    return Atomic_TestAndSet( ( uint32 * ) & psLock->m_nLock );
}

#endif /* __ARCH_X86_SPINLOCK_H__ */

