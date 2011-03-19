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

// Initial kernel process page struct
sProcessPages_t g_psPP_Kernel;
uint32 g_nPP_KernelPTEs[ MM_PP_KERNELPTEs ];

// Lock
sLock_t l_sMemoryLock;

void MM_LockInit ()
{
    Lock_Init ( &l_sMemoryLock );
}

void MM_Lock ()
{
    Lock_Lock ( &l_sMemoryLock );
}

void MM_Unlock ()
{
    Lock_Unlock ( &l_sMemoryLock );
}

void MM_Init ( sMBInfo_t * psMBInfo )
{

    // See if there is a multiboot structure, if not error
    if ( !( ( psMBInfo->m_nFlags ) & ( 1 << 6 ) ) ) {
        Syslog_Lock ();
        Syslog_Entry ( "MM", "FATAL: No multiboot memory map information available - Halting\n" );
        Syslog_Unlock ();
        halt ();
    }

    // Initialise locking
    MM_LockInit ();

    // Calculate Memory in 4K pages, we use the passed multiboot structure
    // to correctly allow for holes in memory
    int nNumsMMapEntries =
        ( psMBInfo->m_nMMAPLength ) / sizeof ( sMBMemoryMap_t );
    sMBMemoryMap_t *sMMap = ( sMBMemoryMap_t * ) psMBInfo->m_nMMAPAddr;
    uint32 nPages = 0;

    // Loop through the memory regions first and do a count of the total
    // number of entries entries required, we can then allocate the required space
    uint32 i;
    for ( i = 0; i < nNumsMMapEntries; i++ ) {

        // Make sure its a type 0x1, i.e. RAM
        if ( sMMap->m_nType == 0x1 ) {
            uint32 nAddr =
                ( sMMap->m_nBaseAddrHigh << 8 ) +
                sMMap->m_nBaseAddrLow;
            uint32 nLen =
                ( ( sMMap->m_nLengthHigh << 8 ) +
                  sMMap->m_nLengthLow ) >> MM_PAGE_OFFSETBITS;

            // Check that the memory is on a page boundary, if not fail
            if ( ( nAddr & MM_PAGE_OFFSETMASK ) != 0 ) {
                Syslog_Lock ();
                Syslog_Entry ( "MM", "RAM at 0x%X, %lu Kb (%lu Pages), not on page boundary. Halting\n",
                               nAddr, nLen << 2, nLen );
                Syslog_Unlock ();
                halt ();
            }
            nPages += nLen;
            Syslog_Lock ();
            Syslog_Entry ( "MM", "RAM at 0x%X, %lu Kb (%lu Pages)\n",
                           nAddr, nLen << 2, nLen );
            Syslog_Unlock ();
        }
        sMMap = ( sMBMemoryMap_t * ) ( ( uint32 ) sMMap +
                                       ( sMMap->m_nSize +
                                         ( uint32 ) sizeof ( sMMap->
                                                             m_nSize ) ) );
    }
    g_sKernel.m_nMemory_SizePages = nPages;
    g_sKernel.m_nMemory_SizeKB = nPages << 2;
    Syslog_Lock ();
    Syslog_Entry ( "MM", "Kernel ends at 0x%X\n", g_sKernel.m_nKernel_End );
    Syslog_Entry ( "MM", "Size of memory is %lu Kb (%lu Pages)\n",
                   g_sKernel.m_nMemory_SizeKB,
                   g_sKernel.m_nMemory_SizePages );
    Syslog_Unlock ();

    // Align free page stack to page boundary
    nPages = ( ( nPages * sizeof ( sFreePageStack_t ) ) +
               ( MM_PAGE_SIZE - 1 ) ) >> MM_PAGE_OFFSETBITS;

    // Now we need to create a free page stack, this contains all the available pages
    // In logical address space, it sits right after the kernel space at 0x10000000
    // Physically is sits at 0x300000
    g_sKernel.m_psMemory_FPS = ( sFreePageStack_t * ) g_sKernel.m_nKernel_End;
    void *pFPSPhysical = ( void * ) MM_KERNEL_FPSPHYSICAL;
    void *pFPSPhysicalEnd = pFPSPhysical + ( nPages << MM_PAGE_OFFSETBITS );
    MM_Page_Map ( pFPSPhysical, g_sKernel.m_psMemory_FPS, nPages, false, true );
    g_sKernel.m_nKernel_End += nPages << MM_PAGE_OFFSETBITS;

    // Now actually store address of each free page in the free page stack
    // We do this by going through the multiboot memory map again, making sure we exclude
    // any regions of physical memory already being used. Straightforward enough.
    g_sKernel.m_nMemory_FPSIndex = 0;
    sMMap = ( sMBMemoryMap_t * ) psMBInfo->m_nMMAPAddr;
    for ( i = 0; i < nNumsMMapEntries; i++ ) {

        // Make sure its a type 0x1, i.e. RAM
        if ( sMMap->m_nType == 0x1 ) {
            uint32 nAddr =
                ( sMMap->m_nBaseAddrHigh * 256 ) +
                sMMap->m_nBaseAddrLow;
            uint32 nLen =
                ( ( sMMap->m_nLengthHigh * 256 ) +
                  sMMap->m_nLengthLow ) >> MM_PAGE_OFFSETBITS;

            // Now loop through all the pages in this list, excluding any as necessary
            uint32 j;
            for ( j = 0; j < nLen; j++ ) {

                // Kernel itself
                if ( ( nAddr >= MM_KERNEL_ADDRESSPHYSICAL )
                        && ( nAddr <
                             ( MM_KERNEL_ADDRESSPHYSICAL +
                               ( g_sKernel.m_nKernel_Size <<
                             MM_PAGE_OFFSETBITS ) ) ) ) {}
                // Stack space, page directory and PT map
                else if ( ( nAddr >= 0x200000 )
                      && ( nAddr < MM_KERNEL_STACKPHYSICALEND ) ) {}
                // Free page stack
                else if ( ( nAddr >= ( uint32 ) pFPSPhysical )
                          && ( nAddr <
                           ( uint32 ) pFPSPhysicalEnd ) ) {}
                // It's valid, so lets add it
                else {
                    sFreePageStack_t * psFreePageEntry = &g_sKernel.m_psMemory_FPS[ g_sKernel.m_nMemory_FPSIndex++ ];
                    psFreePageEntry->m_pPhysicalAddr = ( void * ) nAddr;
                }
                nAddr += MM_PAGE_SIZE;
            }
        }
        sMMap = ( sMBMemoryMap_t * ) ( ( unsigned long ) sMMap +
                                       ( sMMap->m_nSize +
                                         sizeof ( sMMap->m_nSize ) ) );
    }
    // Drop down to point to first free page stack entry
    g_sKernel.m_nMemory_FPSIndex--;

    // Show size and location of free page stack
    Syslog_Lock ();
    Syslog_Entry ( "MM", "Free page stack from 0x%X - 0x%X\n", g_sKernel.m_psMemory_FPS, g_sKernel.m_nKernel_End );
    Syslog_Entry ( "MM", "  -> Size is %lu Kb (%lu Pages)\n", nPages << 2, nPages );
    Syslog_Unlock ();

    // Setup Heap, We don't need to map any memory, the page exceptions will do that
    g_sKernel.m_nMemory_HeapSizePages = MM_KERNEL_HEAPSIZE;
    g_sKernel.m_pHeapStart = ( void * ) MM_KERNEL_HEAP;
    g_sKernel.m_pHeapEnd = ( void * ) MM_KERNEL_HEAPEND;
    MM_AllocInit ( &g_sKernel.m_psMemory_Heap, g_sKernel.m_pHeapStart,
                   g_sKernel.m_nMemory_HeapSizePages );
    Syslog_Lock ();
    Syslog_Entry ( "MM", "Kernel heap from 0x%X - 0x%X\n",
                   g_sKernel.m_pHeapStart, g_sKernel.m_pHeapEnd );
    Syslog_Entry ( "MM", "  -> Size is %lu Kb (%lu Pages)\n",
                   g_sKernel.m_nMemory_HeapSizePages << 2,
                   g_sKernel.m_nMemory_HeapSizePages );
    Syslog_Unlock ();

    // Setup physical memory map area (for drivers etc)
    Syslog_Lock ();
    Syslog_Entry ( "MM", "Physical memory map area from 0x%X - 0x%X\n", MM_KERNEL_PMM, MM_KERNEL_PMMEND );
    Syslog_Entry ( "MM", "  -> Size is %lu KB (%lu Pages)\n",
                   MM_KERNEL_PMMSIZE << 2, MM_KERNEL_PMMSIZE );
    Syslog_Unlock ();

    // Unmap identity-mapped lower 4MB of memory and free the Page table, we don't need it any more
    MM_Page_Unmap ( 0, MM_PAGE_ENTRIESPERPAGE );
    g_sKernel.m_pnKernel_PD[ 0 ] = 0;
    ReloadCR3 ();

    // Do we need to do any post-mm-remap video setup (i.e. remapping logical/physical memory?)
    TextModeVideo_PostMMInit ();

    // Setup initial kernel process page struct and initialise
    g_sKernel.m_psPP_Kernel = &g_psPP_Kernel;
    MM_ZeroPageStruct( g_sKernel.m_psPP_Kernel );
}
/*
void a() {
 
  // Create kernel PTEs space
  g_sKernel.m_pnPP_KernelPTEs = ( uint32 * ) & g_nPP_KernelPTEs;
  for ( i = 0 ; i < MM_PP_KERNELPTEs; i++ ) {
    g_sKernel.m_pnPP_KernelPTEs[ i ] = MM_PP_NOENTRY;
  }
 
  // Add kernel onto kernel process pages
  for ( i = 0; i < g_sKernel.m_nKernel_Size ; i++ ) {
    MM_AddPage( MM_KERNEL_ADDRESS + ( i << MM_PAGE_OFFSETBITS ), MM_KERNEL_ADDRESSPHYSICAL + ( i << MM_PAGE_OFFSETBITS ) );
  }
  uint32 nPagesSoFar = g_sKernel.m_nKernelSize;
 
  // Add page directory
  MM_AddPage( MM_KERNEL_ADDRESS + ( g_sKernel.m_nKernelSize << MM_PAGE_OFFSETBITS ), MM_KERNEL_PDPHYSICAL );
 
  // Add kernel logical PTE
  MM_AddPage( MM_KERNEL_PTMAP + ( MM_KERNEL_ADDRESS >> 10 ), MM_KERNEL_CPTPHYSICAL );
 
  // Add kernel stack
  for ( i = 0; i < MM_KERNEL_STACKSIZE ; i++ ) {
    MM_AddPage( ( uint32 ) g_sKernel.m_pnKernel_Stack + ( i << MM_PAGE_OFFSETBITS ),
                MM_KERNEL_STACKPHYSICAL + ( i << MM_PAGE_OFFSETBITS ) );
  }
  nPagesSoFar += MM_KERNELSTACK;
 
  // Add free page stack pages themselves
  for ( i = 0; i < nPages ; i++ ) {
    MM_AddPage( MM_KERNELADDRPAGED + ( ( 1 + i + g_sKernel.m_nKernelSize ) << MM_PAGESIZESHIFT ), 0x200000 + ( ( MM_KERNELSTACK + 3 + i ) << MM_PAGESIZESHIFT ) );
  }
  nPagesSoFar += nPages;
} 
*/
