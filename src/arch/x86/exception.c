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

static inline void TraceDumpStackWithError ( sCallStackWithErrorCode_t * psStack ) {

  // Dump registers
  Syslog_Entry( NULL, "Pointer: 0x%X\n", psStack );
  Syslog_Entry ( NULL, "EAX: 0x%X  EBX: 0x%X  ECX: 0x%X  EDX: 0x%X\n",
                 psStack->eax, psStack->ebx, psStack->ecx, psStack->edx );
  Syslog_Entry ( NULL, "EBP: 0x%X  ESP: 0x%X  ESI: 0x%X  EDI: 0x%X\n\n",
                 psStack->ebp, psStack->esp, psStack->esi, psStack->edi );

  // Stack Trace
  uint32 *pStack = ( uint32 * ) psStack->esp;
  uint32 n, i;
  for ( i = 0; i < 32; i++ ) {
    n = pStack[ i ];
    Syslog_Entry ( NULL, "0x%08X: 0x%08X", i * 4, n );
    n = pStack[ i++ ];
    Syslog_Entry ( NULL, " 0x%08X", n );
    n = pStack[ i++ ];
    Syslog_Entry ( NULL, " 0x%08X", n );
    n = pStack[ i++ ];
    Syslog_Entry ( NULL, " 0x%08X\n", n );
  }
}

// Fault 1: Debug fault
void Exception_Debug ( sCallStack_t * psStack ) {

  Syslog_Entry ( "EXCEPTION", "Debug Fault from 0x%X:0x%X - Halting\n",
                 psStack->cs, psStack->eip );
}

// Fault 6: Invalid opcode
void Exception_InvalidOpCode ( sCallStack_t * psStack ) {

  Syslog_Entry ( "EXCEPTION",
                 "Invalid Opcode from 0x%X:0x%X - Halting\n",
                 psStack->cs, psStack->eip );
}

// Fault 8: Double fault
void Exception_Double ( sCallStack_t * psStack ) {

  Syslog_Entry ( "EXCEPTION", "Double Fault from 0x%X:0x%X - Halting\n",
                 psStack->cs, psStack->eip );
}

// Fault 12: Stack fault
void Exception_StackFault ( sCallStack_t * psStack ) {

  Syslog_Entry ( "EXCEPTION", "Stack Fault from 0x%X:0x%X - Halting\n",
                 psStack->cs, psStack->eip );
}

// Fault 13: Page fault
void Exception_Page ( sCallStackWithErrorCode_t * psStack ) {

  // Get flags for page, and analyze them
  uint32 nCR2 = MM_PAGE_ALIGN ( GetCR2 () );
  uint32 nPT = MM_GetPTAddress ( ( void * ) nCR2 );
  uint32 nPTOffset = ( nCR2 - nPT ) >> MM_PAGE_OFFSETBITS;
  nPT = nPT >> MM_PAGE_OFFSETBITS;
  uint32 nFlags = g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ];
  if ( nFlags & MM_PAGE_OSPTE_ZEROFILL ) {
    MM_Page_Allocate ( ( void * ) nCR2, true );
  } else {

    // Heap area, if so just allocate a page
    if ( nCR2 >= ( uint32 ) g_sKernel.m_pHeapStart && nCR2 < ( uint32 ) g_sKernel.m_pHeapEnd ) {
      MM_Page_Allocate( ( void * ) nCR2, true );
    } else {
      TraceDumpStackWithError( psStack );
      Syslog_Entry ( "EXCEPTION",
                     "Unhandled Page Fault at 0x%X from 0x%X:0x%X (Error 0x%X, Flags 0x%X)\n",
                     nCR2, psStack->cs, psStack->eip, psStack->error,
                     nFlags );
      halt ();
    }
  }
}

// Fault 14: GPF!
void Exception_GPF ( sCallStackWithErrorCode_t * psStack ) {

  Syslog_Entry ( "EXCEPTION", "GPF from 0x%X:0x%X - Halting\n",
                 psStack->cs, psStack->eip );
  halt ();
}
