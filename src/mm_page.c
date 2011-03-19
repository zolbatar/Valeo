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

// Get the next free page in the free page stack, checking for empty
static inline void *MM_Page_GetFree ( void *pLogicalAddr )
{

    // 0xFFFFFFFF == -1
    if ( g_sKernel.m_nMemory_FPSIndex == 0xFFFFFFFF ) {
        Syslog_Lock ();
        Syslog_Entry ( "MM", "FATAL: No memory available - Halting\n" );
        Syslog_Unlock ();
        halt ();
    }

    // Get physical address of next page of free page stack and return it
    sFreePageStack_t *psFreePageEntry =
        &g_sKernel.m_psMemory_FPS[ g_sKernel.m_nMemory_FPSIndex-- ];
    return psFreePageEntry->m_pPhysicalAddr;
}

// Get the address of the PTE for this logical address, creating new page tables
// in the PD as necessary
uint32 MM_GetPTAddress ( void *pLogicalAddr )
{

    uint32 nPDE = ( uint32 ) pLogicalAddr >> 22;
    uint32 nPT = g_sKernel.m_pnKernel_PD[ nPDE ];

    // See if we need to create a new page table
    if ( !( nPT & MM_PAGE_ARCHPTE_PRESENT ) ) {

        // Get a free page to allocate for the page table
        void * pPTA = ( void * ) MM_KERNEL_PTMAP + ( ( uint32 ) pLogicalAddr >> 10 );
        void * pRealAddr = MM_Page_GetFree ( pPTA );

        // Add to page directory
        g_sKernel.m_pnKernel_PD[ nPDE ] = ( uint32 ) pRealAddr | MM_PAGE_ARCHPTE_PRESENT | MM_PAGE_ARCHPTE_RW;

        // Zero the page table
        memset ( ( void * ) ( &g_sKernel.
                              m_pnKernel_PTMap[ nPDE << 10 ] ), 0,
                 MM_PAGE_SIZE );

        // Invalidate TLB entry
        InvalidatePage ( ( void * ) ( pLogicalAddr ) );
    }
    return ( nPDE << 22 );
}

// Get page table address for specificed logical address
uint32 MM_GetPTValue ( void *pLogicalAddr )
{

    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;
    return g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ];
}

// Simply maps nPageCount number of pages of physical memory contigously
// into the logical address space
void MM_Page_Map ( void *pPhysicalAddr, void *pLogicalAddr, uint32 nPageCount, bool bUser, bool bRW )
{

    // Get page table address and offset
    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;

    // Setup flags
    uint32 nFlags = MM_PAGE_ARCHPTE_PRESENT;
    if ( bUser )
        nFlags |= MM_PAGE_ARCHPTE_USER;
    if ( bRW )
        nFlags |= MM_PAGE_ARCHPTE_RW;

    // Loop through and map all pages
    uint32 i;
    for ( i = 0; i < nPageCount; i++ ) {

        // Check we haven't moved to a new page table entry
        if ( nPTOffset == MM_PAGE_ENTRIESPERPAGE ) {
            nPT = MM_GetPTAddress ( pLogicalAddr ) >> MM_PAGE_OFFSETBITS;
            nPTOffset = 0;
        }

        // Set page table entry
        g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ] = ( uint32 ) pPhysicalAddr | nFlags;

        // Next page
        nPTOffset++;
        pLogicalAddr += MM_PAGE_SIZE;
        pPhysicalAddr += MM_PAGE_SIZE;
    }

    // Reload CR3, might be a better way of doing this though?
    ReloadCR3 ();
}

// Similar to MapPage, but this one marks it as ZeroFill, so the first time the page is
// accessed, a page fault will occur, this should then get a free physical page and zero
// fill it
void MM_Page_MapZeroFill ( void *pLogicalAddr, uint32 nPageCount, bool bUser, bool bRW )
{

    // Get page table address and offset
    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;

    // Setup flags
    uint32 nFlags = MM_PAGE_OSPTE_ZEROFILL;
    if ( bUser )
        nFlags |= MM_PAGE_ARCHPTE_USER;
    if ( bRW )
        nFlags |= MM_PAGE_ARCHPTE_RW;

    // Loop through and zero map all pages
    uint32 i;
    for ( i = 0; i < nPageCount; i++ ) {

        // Check we haven't moved to a new page table entry
        if ( nPTOffset == MM_PAGE_ENTRIESPERPAGE ) {
            nPT = MM_GetPTAddress ( pLogicalAddr ) >> MM_PAGE_OFFSETBITS;
            nPTOffset = 0;
        }

        // Set page table entry
        g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ] = nFlags;

        // Next page
        nPTOffset++;
        pLogicalAddr += MM_PAGE_SIZE;
    }

    // Reload CR3, might be a better way of doing this though?
    ReloadCR3 ();
}

// This simply zeroes the entries for the specified logical addresses, does not free
// up any allocated pages etc.
void MM_Page_Unmap ( void *pLogicalAddr, uint32 nPageCount )
{

    // Get page table address and offset
    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;

    // Loop through and unmap all pages
    uint32 i;
    for ( i = 0; i < nPageCount; i++ ) {

        // Check we haven't moved to a new page table entry
        if ( nPTOffset == MM_PAGE_ENTRIESPERPAGE ) {
            nPT = MM_GetPTAddress
                  ( pLogicalAddr ) >> MM_PAGE_OFFSETBITS;
            nPTOffset = 0;
        }

        // Set page table entry
        g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ] = 0;

        // Next page
        nPTOffset++;
        pLogicalAddr += MM_PAGE_SIZE;
    }

    // Reload CR3, might be a better way of doing this though?
    ReloadCR3 ();
}

// This gets a spare page from the free page stack and allocates at the specified
// logical address.
void MM_Page_Allocate ( void *pLogicalAddr, bool bZeroFill )
{

    // Get page table address and offset
    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;

    // Set page table entry to point to free physical page
    g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ] =
        ( uint32 ) MM_Page_GetFree ( pLogicalAddr ) | MM_PAGE_ARCHPTE_PRESENT | MM_PAGE_ARCHPTE_RW;

    // Do we need to zero fill?
    if ( bZeroFill )
        memset ( ( void * )
                 ( pLogicalAddr ), 0, MM_PAGE_SIZE );

    // Invalidate TLB entry
    InvalidatePage ( ( void * ) ( pLogicalAddr ) );
}

// Free a previously allocated page. This function will add the page back onto the free page
// stack as well
void MM_Page_Free ( void *pLogicalAddr, uint32 nPageCount )
{

    // Get page table address and offset
    uint32 nPT = MM_GetPTAddress ( pLogicalAddr );
    uint32 nPTOffset = ( ( uint32 ) pLogicalAddr - nPT ) >> MM_PAGE_OFFSETBITS;
    nPT = nPT >> MM_PAGE_OFFSETBITS;

    // Loop through and free all zero map pages
    uint32 i;
    for ( i = 0; i < nPageCount; i++ ) {

        // Check we haven't moved to a new page table entry
        if ( nPTOffset == MM_PAGE_ENTRIESPERPAGE ) {
            nPT = MM_GetPTAddress
                  ( pLogicalAddr ) >> MM_PAGE_OFFSETBITS;
            nPTOffset = 0;
        }

        // Is page present? (if not we are not interested)
        if ( g_sKernel.
                m_pnKernel_PTMap[ nPT +
                                  nPTOffset ] & MM_PAGE_ARCHPTE_PRESENT ) {

            // Get physical address of page
            uint32 nPhysicalPage =
                g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ];

            // If not aligned, halt
            if ( ( nPhysicalPage & MM_PAGE_OFFSETMASK ) > 0 ) {
                Syslog_Lock ();
                Syslog_Entry ( "MM", "MM_Page_Free, Physical=0x%X not on page boundary. Halting\n",
                               nPhysicalPage );
                Syslog_Unlock ();
                halt ();
            }

            // Add page to free page stack
            MM_Page_AddFree ( ( void * ) ( nPhysicalPage & MM_PAGE_MASK ) );

            // Set page table entry
            g_sKernel.m_pnKernel_PTMap[ nPT + nPTOffset ] ^= MM_PAGE_ARCHPTE_PRESENT;
        }

        // Next page
        nPTOffset++;
        pLogicalAddr += MM_PAGE_SIZE;
    }

    // Reload CR3, might be a better way of doing this though?
    ReloadCR3 ();
}

// Add physical page specified back onto the free page stack
void MM_Page_AddFree ( void *pPhysicalAddr )
{

    sFreePageStack_t * psFreePageEntry =
        &g_sKernel.m_psMemory_FPS[ ++g_sKernel.m_nMemory_FPSIndex ];
    psFreePageEntry->m_pPhysicalAddr = pPhysicalAddr;
}
