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

bool Timer_Handler( sCallStack_t *psStack );

// Lock
sLock_t sTimerLock;

void Timer_LockInit()
{
    Lock_Init( &sTimerLock );
}

void Timer_Lock()
{
    Lock_Lock( &sTimerLock );
}

void Timer_Unlock()
{
    Lock_Unlock( &sTimerLock );
}

void Timer_Init()
{

    // Initialise locking
    Timer_LockInit();

    // Set no timers
    g_sKernel.m_psLastTimer = NULL;

    // Set timer count to 0;
    g_sKernel.m_nTimer = 0;

    // Attach timer to the PIT and enable IRQ
    Interrupt_AttachHandler( 0, Timer_Handler );
    Interrupt_EnableIRQ( 0x0 );
}

void Timer_AttachTimerHandler( uint32 nTimeOut, TimerHandler *pHandler )
{

    // Lock
    Timer_Lock();

    // Create timer struct
    sTimer_t * psTimer = ( sTimer_t * ) MM_KAlloc( sizeof( sTimer_t ) );
    psTimer->m_nTimeOut = nTimeOut;
    psTimer->m_nTimeOutLeft = nTimeOut;
    psTimer->m_pHandler = pHandler;

    // Add to linked list
    if ( g_sKernel.m_psLastTimer == NULL ) {
        psTimer->m_psPrevious = NULL;
    } else {
        sTimer_t *psPrevious = g_sKernel.m_psLastTimer;
        psTimer->m_psPrevious = psPrevious;
    }
    g_sKernel.m_psLastTimer = psTimer;

    // Release
    Timer_Unlock();
}

bool Timer_Handler( sCallStack_t *psStack )
{

    // Hold
    Timer_Lock();

    // Search for handler
    sTimer_t * psTimer = g_sKernel.m_psLastTimer;
    while ( psTimer != NULL ) {

        TimerHandler * psTimerHandler = psTimer->m_pHandler;
        if ( psTimer->m_nTimeOut == 0 ) {

            // Call handler
            psTimerHandler( psStack );

        } else {

            // We have a timeout based timer
            psTimer->m_nTimeOutLeft--;

            if ( psTimer->m_nTimeOutLeft == 0 ) {
                psTimer->m_nTimeOutLeft = psTimer->m_nTimeOut;
                psTimerHandler( psStack );
            }
        }
        psTimer = psTimer->m_psPrevious;
    }
    Timer_Unlock();

    // Return that we have handled the interrupt
    return true;
}
