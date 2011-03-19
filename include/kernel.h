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

#ifndef __KERNEL_H__
#define __KERNEL_H__

/* Show various debug prompts */
#define DEBUG

/* Standard C includes */
#include <sys/io.h>
#include <sys/types.h>
#include <printf.h>
#include <string.h>

/* Kernel return type */
typedef uint32 KStatus;

/* Kernel variables */
extern bool g_bKernelInitDone;

/* x86 Specific includes */
#include <arch/x86/descriptor.h>
#include <arch/x86/processor.h>
#include <arch/x86/spinlock.h>
#include <arch/x86/8254.h>
#include <arch/x86/8259a.h>
#include <arch/x86/textmodevideo.h>

/* Boot module structure */
#define K_BOOTMODULES_MAX 8
typedef void BootEntry ( void );
typedef struct {
  void *m_pStart;
  void *m_pEnd;
  uint32 m_nLength;
  BootEntry *m_pEntry;
  void *m_pLoadedAddress;
  void *m_pLoadedAddressEnd;
  uint32 m_nLoadedAddressPages;
}
sBootModule_t;

/* Kernel includes */
#include <locking.h>
#include <multiboot.h>
#include <interrupt.h>
#include <mm.h>
#include <syslog.h>
#include <resource.h>
#include <timer.h>
#include <ipc.h>

/* Version information */
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_REVISION 1
#define VERSION_NAME "Valeo (Sirus)"

/* Main kernel structure, this is heavily used throughout */
typedef struct {
  /* Microkernel */
  uint32 m_nKernel_Size;
  void *m_nKernel_End;
  uint32 *m_pnKernel_Stack;
  uint32 *m_pnKernel_PD;
  uint32 *m_pnKernel_PTMap;

  /* Memory management */
  uint32 m_nMemory_SizePages;
  uint32 m_nMemory_SizeKB;
  sFreePageStack_t *m_psMemory_FPS;
  uint32 m_nMemory_FPSIndex;
  uint32 m_nMemory_HeapSizePages;
  void *m_pHeapStart;
  void *m_pHeapEnd;
  sMMAllocator_t m_psMemory_Heap;

  /* Process pages */
  sProcessPages_t *m_psPP_Kernel;
  uint32 *m_pnPP_KernelPTEs;

  /* Boot modules */
  uint32 m_nBootModuleCount;
  void *m_pBootModuleStart;
  void *m_pBootModuleEnd;
  sBootModule_t m_sBootModules[ K_BOOTMODULES_MAX ];

  /* Timers */
  sTimer_t *m_psLastTimer;
  uint64 m_nTimer;
}
sKernel_t;
extern sKernel_t g_sKernel;

/* This function is in Boot.S and halts the system */
extern void halt ( void );

#endif /* __KERNEL_H__ */
