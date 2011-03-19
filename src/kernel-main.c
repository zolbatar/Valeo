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

// Globals from Boot.S
extern uint32 KernelSize;
extern uint32 *PageDirectory;
extern sIDT_t idt;
extern sGDT_t gdt;

// Globals
sKernel_t g_sKernel;
bool g_bQuietMode = false;
bool g_bKernelInitDone = false;

// Version
uint32 l_nVersionMajor = VERSION_MAJOR;
uint32 l_nVersionMinor = VERSION_MINOR;
uint32 l_nVersionRevision = VERSION_REVISION;
char l_nVersionName[] = { VERSION_NAME };

// Prototypes
void ModuleInitThread ( void );
void Kernel_ParseParameters ( char *pszParameters );
void Kernel_ParseBootModules ( sMBInfo_t * psMBInfo );
void Kernel_LoadBootModules ( void );

void Kernel_Main ( sMBInfo_t * psMBInfo )
{

    // Get values from boot.S and put in the g_sKernel struct
    g_sKernel.m_nKernel_Size = KernelSize;

    // Set address of page directory
    g_sKernel.m_pnKernel_PD = ( void * ) MM_KERNEL_ADDRESS + ( g_sKernel.m_nKernel_Size << MM_PAGE_OFFSETBITS );

    // Set address of stack
    g_sKernel.m_pnKernel_Stack = ( void * ) MM_KERNEL_ADDRESS +
                                 ( ( g_sKernel.m_nKernel_Size + 1 ) << MM_PAGE_OFFSETBITS );

    // Calculate kernel end = Base Address
    // + Kernel Size
    // + Kernel Stack
    // + Page Directory
    g_sKernel.m_nKernel_End = ( void * ) MM_KERNEL_ADDRESS +
                              ( ( g_sKernel.m_nKernel_Size + MM_KERNEL_STACKSIZE + 1 ) << MM_PAGE_OFFSETBITS );
    g_sKernel.m_pnKernel_PTMap = ( uint32 * ) MM_KERNEL_PTMAP;

    // Setup text mode for debugging and start the system log
    TextModeVideo_Init ();
    Syslog_Init ();

    // Setup IDT/GDT structures
    Descriptor_Init ( &idt, &gdt );

    // Parse boot parameters
    Syslog_Entry ( "MICROKERNEL", "Starting...\n" );
    Kernel_ParseParameters ( ( char * ) psMBInfo->m_nCmdLine );

    /* Parse boot modules (memory location etc. for later) */
    //    Kernel_ParseBootModules( psMBInfo );

    // Initialise core functions
    Interrupt_Init ();
    MM_Init ( psMBInfo );
    Resource_Init();
    PIC_Init ();
    PIT_Init ();
    Timer_Init ();

    // Show how many free pages after kernel initialisation
    Syslog_Entry ( "MM", "Free memory of %lu Kb (%lu Pages)\n",
                   g_sKernel.m_nMemory_FPSIndex << 2,
                   g_sKernel.m_nMemory_FPSIndex );

    // Now start interrupts and enable multi-tasking etc
    g_bKernelInitDone = true;
    asm volatile ( "sti;" );

    // Kernel idle loop, basically to wait for an interrupt if nothing to run
    while ( 1 ) {
        asm volatile ( "hlt;" );
    }

    // Shutdown
    Syslog_Lock ();
    Syslog_Entry ( "MICROKERNEL", "Halted now.\n" );
    Syslog_Unlock ();
    halt ();
}

void Kernel_ParseParameters ( char *pszParameters )
{
    // First off, scan until either the end or first space (get rid of kernel filename)
    uint32 i = 0;
    while ( pszParameters[ i ] != '\0' && !isspace ( pszParameters[ i ] ) )
        i++;

    // If we have some parameters, process
    if ( pszParameters[ i ] != '\0' ) {

        bool bLast = false;
        uint32 nEnd;
        do {
            // Dump leading spaces
            while ( isspace ( pszParameters[ i ] ) )
                i++;

            // Scan for end of command
            nEnd = i;
            while ( pszParameters[ nEnd ] != '\0'
                    && !isspace ( pszParameters[ nEnd ] ) )
                nEnd++;
            if ( pszParameters[ nEnd ] == '\0' ) {
                bLast = true;
            }

            // Now process command
            if ( strncmp ( &pszParameters[ i ], "quiet", 5 ) == 0 ) {
                g_bQuietMode = true;
            }

            // Next
            i = nEnd;

        } while ( bLast == false );
    }
}

/*
void Kernel_ParseBootModules( sMBInfo_t *psMBInfo ) {
 
    // Init kernel variables
    g_sKernel.m_nBootModuleCount = psMBInfo->m_nModsCount;
    g_sKernel.m_pBootModuleStart = ( void * ) - 1;
    g_sKernel.m_pBootModuleEnd = 0;
 
    // Too many boot modules?
    if ( g_sKernel.m_nBootModuleCount > K_BOOTMODULES_MAX ) {
        Syslog_Lock();
        Syslog_Entry( "MICROKERNEL", "Too many boot modules - Halting\n" );
        Syslog_Unlock();
        halt();
    }
 
    // Show how many modules
    Syslog_Lock();
    Syslog_Entry( "MICROKERNEL", "%lu boot module(s) found\n", g_sKernel.m_nBootModuleCount );
    Syslog_Unlock();
 
    // Now move modules out the way, we will come back to them later and load them in
    uint32 i;
    sMBModule_t *psModule = ( sMBModule_t * ) psMBInfo->m_nModsAddr;
    for ( i = 0; i < g_sKernel.m_nBootModuleCount; i++ ) {
 
        // Get position etc and save for later
        sBootModule_t *psKernelModule = &g_sKernel.m_sBootModules[ i ];
        psKernelModule->m_pStart = ( void * ) psModule->m_nStart;
        psKernelModule->m_pEnd = ( void * ) psModule->m_nEnd;
        psKernelModule->m_nLength = psModule->m_nEnd - psModule->m_nStart;
 
        if ( psKernelModule->m_pStart < g_sKernel.m_pBootModuleStart ) {
            g_sKernel.m_pBootModuleStart = psKernelModule->m_pStart;
        }
 
        // Move end of modules in physical memory pointer along
        g_sKernel.m_pBootModuleEnd = ( void * ) ( MM_PAGEALIGN( psKernelModule->m_pEnd + ( MM_PAGESIZE - 1 ) ) );
 
        // Debug prompt
        Syslog_Lock();
        Syslog_Entry( "MICROKERNEL", "Boot module found at 0x%X-0x%X, %lu bytes\n",
                      psKernelModule->m_pStart, psKernelModule->m_pEnd, psKernelModule->m_nLength, g_sKernel.m_pBootModuleEnd );
        Syslog_Unlock();
 
        // Next module
        psModule = ( sMBModule_t * ) ( ( uint32 ) psModule + ( uint32 ) sizeof( sMBModule_t ) );
    }
    Syslog_Lock();
    Syslog_Entry( "MICROKERNEL", "Boot modules at 0x%X-0x%X\n", g_sKernel.m_pBootModuleStart, g_sKernel.m_pBootModuleEnd );
    Syslog_Unlock();
}
 
void Kernel_LoadBootModules() {
 
    // Loop through all modules and map them into the virtual address space
    uint32 i;
    for ( i = 0; i < g_sKernel.m_nBootModuleCount; i++ ) {
        OS_ModuleEntry *pGetDefinition;
        sBootModule_t *psKernelModule = &g_sKernel.m_sBootModules[ i ];
        LoadELFBootModule( psKernelModule, i, ( void * ) & pGetDefinition );
 
        // Clear interrupts for a short while
        asm volatile ( "cli;" );
 
        // Add the memory pages used during grub load to the free page stack for later use
        uint32 nPage = ( uint32 ) psKernelModule->m_pStart;
        uint32 nPageCount = ( psKernelModule->m_nLength + ( MM_PAGESIZE - 1 ) ) >> MM_PAGESIZESHIFT;
        uint32 i;
        MM_Lock();
        for ( i = 0 ; i < nPageCount; i++ ) {
            MM_Page_AddFree( nPage += MM_PAGESIZE );
        }
        MM_Unlock();
 
        // Get the module definition
        OS_sModuleDefinition_t *pDefinition = pGetDefinition();
 
        // Get main() and enter
        OS_ModuleMain *pEntry = pDefinition->m_pEntryPoint;
        KStatus nReturn = pEntry();
 
        // Did it load successfully?, if not unload it.
        if ( nReturn == 0 ) {
            MM_Page_Free( ( uint32 ) psKernelModule->m_pLoadedAddress, psKernelModule->m_nLoadedAddressPages );
        }
 
        // Add to module list
        Module_Add( pGetDefinition, psKernelModule->m_nLoadedAddressPages, pDefinition );
 
        // And restart interrupts
        asm volatile ( "sti;" );
    }
}
*/
