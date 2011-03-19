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

#ifndef __LOCKING_H__
#define __LOCKING_H__

/* Locking struct */
typedef struct {
  sSpinlock_t m_sSpinlock;
}
sLock_t;

/* Init lock */
static inline void Lock_Init( sLock_t *psLock ) {
  Spinlock_Init( &psLock->m_sSpinlock );
}

static inline void Lock_Lock( sLock_t *psLock ) {

  /* Only need locks if multi-tasking enabled */
  if ( g_bKernelInitDone == true ) {
    Spinlock_Lock( &psLock->m_sSpinlock );
  }
}

static inline bool Lock_TryLock( sLock_t *psLock ) {

  //*Only need locks if multi-tasking enabled */
  if ( g_bKernelInitDone == true ) {
    return Spinlock_TryLock( &psLock->m_sSpinlock );
  } else {
    return false;
  }
}

static inline void Lock_Unlock( sLock_t * psLock ) {

  /* Only need locks if multi-tasking enabled */
  if ( g_bKernelInitDone == true ) {
    Spinlock_Unlock( &psLock->m_sSpinlock );
  }
}

#endif /* __LOCKING_H__ */
