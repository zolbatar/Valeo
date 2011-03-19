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

void MM_ZeroPageStruct( sProcessPages_t *psProcessPages ) {

  /* Zero used map */
  psProcessPages->m_nMap = 0;

  /* Zero all entries */
  memset ( ( void * ) & psProcessPages->m_nPhysicalAddr, 0, sizeof( psProcessPages->m_nPhysicalAddr ) );
  memset ( ( void * ) & psProcessPages->m_nVirtualAddr, 0, sizeof( psProcessPages->m_nVirtualAddr ) );
}
/*
void MM_AddPageToPageStruct( sProcessPages_t *psProcessPages, uint32 nVirtualAddr, uint32 nPhysicalAddr ) {

  // Find an empty struct
  bool bFound = false;
  while ( !bFound ) {

    /. If full, create another one
    if ( psProcessPages->m_nMap == MM_PROCESSPAGESTRUCTFULL ) {

      // Is this valid? If not, create another one, otherwise move to next
      if ( psProcessPages->m_psNext == NULL ) {
        sProcessPages_t * psProcessPagesNew = MM_KAlloc( sizeof( sProcessPages_t ) );
        MM_ZeroPageStruct( psProcessPagesNew );
        psProcessPages->m_psNext = psProcessPagesNew;
      }

      // Move to next
      psProcessPages = psProcessPages->m_psNext;

    } else {
      bFound = true;
    }
  }

  // Now find an empty slot
  uint32 i;
  for ( i = 0; i < MM_ENTRIESPERPROCESSPAGESTRUCT; i++ ) {
    if ( ( psProcessPages->m_nMap & ( 1 << i ) ) == 0 ) {
      psProcessPages->m_nMap ^= ( 1 << i );
      psProcessPages->m_nVirtualAddr[ i ] = nVirtualAddr;
      psProcessPages->m_nPhysicalAddr[ i ] = nPhysicalAddr;
      break;
    }
  }
}

void MM_AddPage( uint32 nVirtualAddr, uint32 nPhysicalAddr ) {

  // What list to process page list add it on
  if ( nVirtualAddr == MM_PAGENOTMAPPED ) {

    // Not mapped into memory
    MM_AddPageToPageStruct( g_sKernel.m_psKernelProcessPages, nVirtualAddr, nPhysicalAddr );

  } else if ( nVirtualAddr >= MM_KERNELADDRPAGED ) {

    if ( nVirtualAddr >= MM_PAGETABLES && nVirtualAddr < MM_PAGETABLESEND ) {

//
      uint32 nIndex = ( nVirtualAddr - MM_PAGETABLES ) >> MM_PAGESIZESHIFT;

      if ( nIndex >= MM_USERPTEs ) {

        // Add to kernel PTEs
        g_sKernel.m_pnKernelPTEs[ nIndex - MM_USERPTEs ] = nPhysicalAddr;

      } else {

        //
        Syslog_Entry( "MM", "User PTE @ 0x%X (0x%X) %lu. Halting.\n", nVirtualAddr, nPhysicalAddr, nIndex );
        halt();

      }

    } else {
      MM_AddPageToPageStruct( g_sKernel.m_psKernelProcessPages, nVirtualAddr, nPhysicalAddr );
    }
  } else {
    Syslog_Entry( "MM", "User address space @ 0x%X (0x%X). Halting.\n", nVirtualAddr, nPhysicalAddr );
    halt();
  }
}*/
