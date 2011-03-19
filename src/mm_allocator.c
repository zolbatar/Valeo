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

void *MM_KAlloc ( uint32 nSize )
{
    return MM_Alloc( nSize, &g_sKernel.m_psMemory_Heap );
}

void MM_KFree ( void *pAllocation )
{
    MM_Free( pAllocation, &g_sKernel.m_psMemory_Heap );
}

void *MM_KExpand ( void *pAllocation, uint32 nAdditionalSize )
{
    return MM_Expand( pAllocation, nAdditionalSize, &g_sKernel.m_psMemory_Heap );
}

void MM_AllocInit ( sMMAllocator_t * psAllocator, void *pAddress, uint32 nPages )
{
    // Setup allocator struct
    psAllocator->m_pHeap = pAddress;
    psAllocator->m_nPages = nPages;

    // Create first empty malloc entry, this is start of linked list
    sMMAlloc_t *psAlloc = psAllocator->m_psAllocHead =
                              ( sMMAlloc_t * ) pAddress;
    psAlloc->m_nSize = 0;
    psAlloc->m_psNext = NULL;

    // Create first heap free map entry, this is start of linked list
    sMMFreeRegion_t *psHeapFreeEntry = psAllocator->m_psFreeHead =
                                           ( sMMFreeRegion_t * ) ( ( uint32 ) pAddress +
                                                                   ( uint32 ) sizeof ( sMMAlloc_t ) );
    psHeapFreeEntry->m_nSize =
        ( nPages << MM_PAGE_OFFSETBITS ) -
        ( uint32 ) sizeof ( sMMFreeRegion_t ) -
        ( uint32 ) sizeof ( sMMAlloc_t );
    psHeapFreeEntry->m_psNext = NULL;
}

void *MM_Alloc ( uint32 nSize, sMMAllocator_t * psAllocator )
{
    // How much space do we need? (word align)
    uint32 nSizeNeeded = ( ( nSize + sizeof ( sMMAlloc_t ) + 3 ) >> 2 ) << 2;

    // Start at beginning of heap free linked list
    sMMFreeRegion_t *psFreeEntry = psAllocator->m_psFreeHead;
    sMMFreeRegion_t *psPrevious = NULL;

    // Loop through linked list until we find a first fit
    while ( psFreeEntry != NULL ) {

        // Is this entry big enough
        if ( psFreeEntry->m_nSize >= nSizeNeeded ) {

            // Move the free space structure forward ahead of this allocation
            sMMFreeRegion_t * psNewFreeEntry =
                ( sMMFreeRegion_t * ) ( ( uint32 ) psFreeEntry +
                                        nSizeNeeded );
            psNewFreeEntry->m_nSize =
                ( uint32 ) psFreeEntry->m_nSize -
                ( uint32 ) nSizeNeeded;

            // Update pointers on found entry to point to this structure
            if ( psAllocator->m_psFreeHead == psFreeEntry ) {
                psAllocator->m_psFreeHead = psNewFreeEntry;
            } else {
                psPrevious->m_psNext = psNewFreeEntry;
            }
            psNewFreeEntry->m_psNext = psFreeEntry->m_psNext;

            // Convert the original free entry into an allocation and update
            sMMAlloc_t *psAlloc = ( sMMAlloc_t * ) psFreeEntry;
            psAlloc->m_nSize = nSizeNeeded;
            psAlloc->m_psNext = psAllocator->m_psAllocHead;

            // Update allocation struct to point to end of list (i.e. this new alloc)
            psAllocator->m_psAllocHead = psAlloc;

            // Return
            return ( void * ) ( ( uint32 ) psAlloc + ( uint32 ) sizeof ( sMMAlloc_t ) );
        }

        // Move onto next free entry (if there is one)
        psPrevious = psFreeEntry;
        psFreeEntry = psFreeEntry->m_psNext;
    }

    // We can't find an suitable space
    Syslog_Lock ();
    Syslog_Entry ( "MM", "FATAL: Unable to allocate memory (%lu/%lu bytes)- Halting\n",
                   nSize, nSizeNeeded );
    Syslog_Unlock ();
    halt ();
    return NULL;
}

void MM_Free ( void *pAllocation, sMMAllocator_t * psAllocator )
{

    // Find the linked list entry for this allocation
    sMMAlloc_t * psAllocToFind =
        ( sMMAlloc_t * ) ( ( uint32 ) pAllocation -
                           ( uint32 ) sizeof ( sMMAlloc_t ) );
    sMMAlloc_t *psAlloc = psAllocator->m_psAllocHead;
    sMMAlloc_t *psPrevious = NULL;
    while ( psAlloc != psAllocToFind ) {
        if ( psAlloc->m_psNext == 0 )
            break;
        psPrevious = psAlloc;
        psAlloc = psAlloc->m_psNext;
    }

    // Did we find an allocation? if not error, return
    if ( psAlloc->m_psNext == NULL ) {
        Syslog_Lock ();
        Syslog_Entry ( "MM",
                       "FATAL: Invalid free request for allocation at 0x%X\n",
                       pAllocation );
        Syslog_Unlock ();
        halt ();
        return ;
    }

    // Now remove allocation
    sMMAlloc_t *pcNext = psAlloc->m_psNext;
    if ( psPrevious == NULL ) {
        psAllocator->m_psAllocHead = pcNext;
    } else {
        psPrevious->m_psNext = pcNext;
    }

    // Convert this allocation into a free entry, making sure the free
    // spaces are kept in the right order so we can do coalescing of free holes
    sMMFreeRegion_t *psNewFree = ( sMMFreeRegion_t * ) psAlloc;
    sMMFreeRegion_t *psFree = psAllocator->m_psFreeHead;
    sMMFreeRegion_t *psPreviousFree = NULL;
    while ( psFree > psNewFree ) {
        psPreviousFree = psFree;
        psFree = psFree->m_psNext;
    }

    // Are we at beginning?
    if ( psFree == NULL ) {
        psNewFree->m_psNext = NULL;
        psPreviousFree->m_psNext = psNewFree;
    } else {
        psNewFree->m_psNext = psPreviousFree->m_psNext;
        psPreviousFree->m_psNext = psNewFree;
    }

    // See if we need to coalesce
    MM_Coalesce ( psAllocator, psPreviousFree );
}

// This will increase the size of the current allocation
// It works by creating a new allocation, copying the bytes over and
// then freeing the old allocation, simple and effiective */
void *MM_Expand ( void *pAllocation, uint32 nAdditionalSize, sMMAllocator_t * psAllocator )
{

    // Find current allocation
    sMMAlloc_t * psAllocToFind = ( sMMAlloc_t * ) ( ( uint32 ) pAllocation - ( uint32 ) sizeof ( sMMAlloc_t ) );
    sMMAlloc_t *psAlloc = psAllocator->m_psAllocHead;
    sMMAlloc_t *psPrevious = NULL;
    while ( psAlloc != psAllocToFind ) {
        if ( psAlloc->m_psNext == 0 )
            break;
        psPrevious = psAlloc;
        psAlloc = psAlloc->m_psNext;
    }

    // Did we find an allocation? if not error, return
    if ( psAlloc->m_psNext == NULL ) {
        Syslog_Lock ();
        Syslog_Entry ( "MM",
                       "FATAL: Invalid free request for allocation at 0x%X\n",
                       pAllocation );
        Syslog_Unlock ();
        halt ();
        return NULL;
    }

    // Now create a new allocation
    uint32 nNewSize = psAlloc->m_nSize + nAdditionalSize;
    void *pCurPtr =
        ( void * ) ( ( uint32 ) psAlloc + ( uint32 ) sizeof ( sMMAlloc_t ) );
    void *pNewPtr = MM_Alloc ( nNewSize, psAllocator );

    // Copy the contents, old to new
    memcpy ( pNewPtr, pCurPtr, psAlloc->m_nSize );

    // And remove the old allocation
    MM_Free ( pAllocation, psAllocator );

    // And return new ptr
    return pNewPtr;
}

void MM_Coalesce ( sMMAllocator_t * psAllocator, sMMFreeRegion_t *psFree )
{

    // Get some pointers to move through the list
    if ( psFree == NULL ) {
        psFree = psAllocator->m_psFreeHead;
    }
    sMMFreeRegion_t *pcNextFree = psFree->m_psNext;
    sMMFreeRegion_t *psPreviousFree = NULL;

    // Keep going until the end
    while ( pcNextFree != NULL ) {

        // Get end of this free space
        sMMFreeRegion_t * pcNextFreeEnd =
            ( sMMFreeRegion_t * ) ( ( uint32 ) pcNextFree +
                                    ( uint32 ) pcNextFree->m_nSize );

        // See if the next one is next to it
        if ( psFree == pcNextFreeEnd ) {
            pcNextFree->m_nSize += psFree->m_nSize;

            // Is this the first?
            if ( psFree == psAllocator->m_psFreeHead ) {
                psAllocator->m_psFreeHead = pcNextFree;
            }

            // Point previous to this new larger one
            if ( psPreviousFree != NULL ) {
                psPreviousFree->m_psNext = pcNextFree;
            }
            psFree = pcNextFree;
            pcNextFree = psFree->m_psNext;
        } else {
            psPreviousFree = psFree;
            psFree = psFree->m_psNext;
            pcNextFree = psFree->m_psNext;
        }
    }
}

