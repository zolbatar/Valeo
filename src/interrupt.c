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

// Externals
extern sIDTR_t *idtr;
extern void DefaultIntHandler ();

// Pointers to exception handlers
extern void EXC_00 ();
extern void EXC_01 ();
extern void EXC_02 ();
extern void EXC_03 ();
extern void EXC_04 ();
extern void EXC_05 ();
extern void EXC_INVALIDOPCODE ();
extern void EXC_07 ();
extern void EXC_DOUBLEFAULT ();
extern void EXC_09 ();
extern void EXC_0A ();
extern void EXC_0B ();
extern void EXC_STACKFAULT ();
extern void EXC_GPF ();
extern void EXC_PAGEFAULT ();

// Pointers to hardware interrupt handlers
extern void IRQ_00 ();
extern void IRQ_01 ();
extern void IRQ_02 ();
extern void IRQ_03 ();
extern void IRQ_04 ();
extern void IRQ_05 ();
extern void IRQ_06 ();
extern void IRQ_07 ();
extern void IRQ_08 ();
extern void IRQ_09 ();
extern void IRQ_0A ();
extern void IRQ_0B ();
extern void IRQ_0C ();
extern void IRQ_0D ();
extern void IRQ_0E ();
extern void IRQ_0F ();

// IRQ Handlers
static sIRQ_Handler_t *l_IRQ_Handler[ INTERRUPT_NUMIRQS ];

// Prototypes
uint32 Interrupt_Handler ( uint32 nCall, void *pData );

// Lock
sLock_t sInterruptLock;

void Interrupt_LockInit()
{
    Lock_Init( &sInterruptLock );
}

void Interrupt_Lock()
{
    Lock_Lock( &sInterruptLock );
}

void Interrupt_Unlock()
{
    Lock_Unlock( &sInterruptLock );
}

void Interrupt_Init ()
{
    // Initialise locking
    Interrupt_LockInit();

    // Clear all set hardware interrupt handlers
    uint32 i;
    for ( i = 0; i < INTERRUPT_NUMIRQS; i++ ) {
        l_IRQ_Handler[ i ] = NULL;
    }

    for ( i = 0; i < PROCESSOR_NUMINTERRUPTGATES; i++ ) {
        Descriptor_SetInterruptGate ( i, &DefaultIntHandler );
    }

    // Set exception handlers
    Descriptor_SetInterruptGate ( 0x00, &EXC_00 );
    Descriptor_SetInterruptGate ( 0x01, &EXC_01 );
    Descriptor_SetInterruptGate ( 0x02, &EXC_02 );
    Descriptor_SetInterruptGate ( 0x03, &EXC_03 );
    Descriptor_SetInterruptGate ( 0x04, &EXC_04 );
    Descriptor_SetInterruptGate ( 0x05, &EXC_05 );
    Descriptor_SetInterruptGate ( 0x06, &EXC_INVALIDOPCODE );
    Descriptor_SetInterruptGate ( 0x07, &EXC_07 );
    Descriptor_SetInterruptGate ( 0x08, &EXC_DOUBLEFAULT );
    Descriptor_SetInterruptGate ( 0x09, &EXC_09 );
    Descriptor_SetInterruptGate ( 0x0A, &EXC_0A );
    Descriptor_SetInterruptGate ( 0x0B, &EXC_0B );
    Descriptor_SetInterruptGate ( 0x0C, &EXC_STACKFAULT );
    Descriptor_SetInterruptGate ( 0x0D, &EXC_GPF );
    Descriptor_SetInterruptGate ( 0x0E, &EXC_PAGEFAULT );

    // Set Default Interrupt Handlers
    Descriptor_SetInterruptGate ( 0x20, &IRQ_00 );
    Descriptor_SetInterruptGate ( 0x21, &IRQ_01 );
    Descriptor_SetInterruptGate ( 0x22, &IRQ_02 );
    Descriptor_SetInterruptGate ( 0x23, &IRQ_03 );
    Descriptor_SetInterruptGate ( 0x24, &IRQ_04 );
    Descriptor_SetInterruptGate ( 0x25, &IRQ_05 );
    Descriptor_SetInterruptGate ( 0x26, &IRQ_06 );
    Descriptor_SetInterruptGate ( 0x27, &IRQ_07 );
    Descriptor_SetInterruptGate ( 0x28, &IRQ_08 );
    Descriptor_SetInterruptGate ( 0x29, &IRQ_09 );
    Descriptor_SetInterruptGate ( 0x2A, &IRQ_0A );
    Descriptor_SetInterruptGate ( 0x2B, &IRQ_0B );
    Descriptor_SetInterruptGate ( 0x2C, &IRQ_0C );
    Descriptor_SetInterruptGate ( 0x2D, &IRQ_0D );
    Descriptor_SetInterruptGate ( 0x2E, &IRQ_0E );
    Descriptor_SetInterruptGate ( 0x2F, &IRQ_0F );

    // Load IDT
    asm volatile ( "lidtl %0 "::"m" ( idtr ) );
}

void Interrupt_Handle ( uint32 nInterrupt, sCallStack_t * psStack )
{
    // Lock. Is this truly necessary? Or even a good idea...
    Interrupt_Lock();

    // Get handler
    sIRQ_Handler_t * psHandler = l_IRQ_Handler[ nInterrupt ];

    // Do we have any handlers for this interrupt?
    if ( psHandler == NULL ) {
        Syslog_Lock ();
        Syslog_Entry ( "INTERRUPT", "No handler for interrupt %lu\n", nInterrupt );
        Syslog_Unlock ();
    } else {

        // Disable and acknowledge
        PIC_MaskAndAcknowledge ( nInterrupt );

        // Loop through all handlers until one returns true for handled
        bool nRet = false;
        while ( psHandler != NULL ) {
            nRet = psHandler->m_pHandler ( psStack );
            if ( nRet )
                break;
            psHandler = psHandler->m_psPrevious;
        }

        // Did any of the handlers handle it?
        if ( !nRet ) {
            Syslog_Lock ();
            Syslog_Entry ( "INTERRUPT", "Unhandled interrupt %lu\n", nInterrupt );
            Syslog_Unlock ();
        }

        // Re-enable interrupt
        PIC_EnableIRQ ( nInterrupt );
    }

    // Release
    Interrupt_Lock();
}

bool Interrupt_AttachHandler ( uint32 nInterrupt, IRQHandler * pHandler )
{
    // Hold
    Interrupt_Lock();

    // Its critical that we kill interrupts
    if ( g_bKernelInitDone == true ) {
        asm volatile ( "cli;" );
    }

    // Allocate space for handler first
    MM_Lock();
    sIRQ_Handler_t * psHandler = ( sIRQ_Handler_t * ) MM_KAlloc( sizeof( sIRQ_Handler_t ) );
    MM_Unlock();

    // Configure handler struct
    psHandler->m_pHandler = pHandler;

    // Add to linked list
    if ( l_IRQ_Handler[ nInterrupt ] == NULL ) {
        psHandler->m_psPrevious = NULL;

    } else {
        sIRQ_Handler_t *psPrevious = l_IRQ_Handler[ nInterrupt ];
        psHandler->m_psPrevious = psPrevious;

    }
    l_IRQ_Handler[ nInterrupt ] = psHandler;

    // Now its safe to re-enable
    if ( g_bKernelInitDone == true ) {
        asm volatile ( "sti;" );
    }

    // Release
    Interrupt_Unlock();

    return true;
}

bool Interrupt_EnableIRQ ( uint32 nInterrupt )
{
    PIC_EnableIRQ ( nInterrupt );
    return true;
}
