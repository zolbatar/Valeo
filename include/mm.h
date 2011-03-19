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

#ifndef __MM_H__
#define __MM_H__

/* Various memory structure defines, x86 Specific */
#define MM_PAGE_SIZE 4096
#define MM_PAGE_SIZEBITS 20
#define MM_PAGE_OFFSETBITS 12
#define MM_PAGE_MASK 0xFFFFF000
#define MM_PAGE_OFFSETMASK 0x00000FFF
#define MM_PAGE_ALIGN(pAddress) (((uint32)pAddress) & MM_PAGE_MASK)
#define MM_PAGE_ENTRIESPERPAGE 1024

/* Memory structures in physical memory */
#define MM_KERNEL_ADDRESSPHYSICAL 0x100000
#define MM_KERNEL_PDPHYSICAL 0x200000
#define MM_KERNEL_CPTPHYSICAL 0x201000
#define MM_KERNEL_STACKSIZE 16
#define MM_KERNEL_STACKPHYSICAL 0x202000
#define MM_KERNEL_STACKPHYSICALEND (0x202000 + (MM_KERNEL_STACKSIZE * MM_PAGE_SIZE))
#define MM_KERNEL_FPSPHYSICAL 0x300000

/* Memory structures in logical memory */
#define MM_KERNEL_ADDRESS 0xF0000000
#define MM_KERNEL_PTMAP 0xFFC00000
#define MM_KERNEL_HEAP 0xF4000000
#define MM_KERNEL_HEAPEND 0xFFC00000
#define MM_KERNEL_HEAPSIZE ((MM_KERNEL_HEAPEND - MM_KERNEL_HEAP) >> MM_PAGE_OFFSETBITS)
#define MM_KERNEL_CACHE 0xD0000000
#define MM_KERNEL_CACHEEND 0xF0000000
#define MM_KERNEL_CACHESIZE ((MM_KERNEL_CACHEEND - MM_KERNEL_CACHE) >> MM_PAGE_OFFSETBITS)
#define MM_KERNEL_PMM 0xC0000000
#define MM_KERNEL_PMMEND 0xD0000000
#define MM_KERNEL_PMMSIZE ((MM_KERNEL_PMMEND - MM_KERNEL_PMM) >> MM_PAGE_OFFSETBITS)

/* Page entry flags */
#define MM_PAGE_ARCHPTE_PRESENT (1 << 0)
#define MM_PAGE_ARCHPTE_RW (1 << 1)
#define MM_PAGE_ARCHPTE_USER (1 << 2)
#define MM_PAGE_ARCHPTE_ACCESSED (1 << 5)
#define MM_PAGE_ARCHPTE_DIRTY (1 << 6)

/* PTE, bits 9/10/11 are free */
#define MM_PAGE_OSPTE_ZEROFILL (1 << 9)

/* Process page structs */
#define MM_PP_ENTRIES 32
#define MM_PP_KERNELPTEs 512
#define MM_PP_USERPTEs 512
#define MM_PP_FULL 0xFFFFFFFF
#define MM_PP_NOENTRY 0xFFFFFFFF 
//#define MM_PAGENOTMAPPED 0xFFFFFFFF

typedef struct sProcessPages sProcessPages_t;
struct sProcessPages {
  uint32 m_nMap;
  uint32 m_nPhysicalAddr[ MM_PP_ENTRIES ];
  uint32 m_nVirtualAddr[ MM_PP_ENTRIES ];
  sProcessPages_t *m_psNext;
};
extern sProcessPages_t g_sKernelProcessPageStruct;

/* Various MM structs */
typedef struct {
  void *m_pPhysicalAddr;
}
sFreePageStack_t;

typedef struct sMMAlloc sMMAlloc_t;
struct sMMAlloc {
  uint32 m_nSize;
  sMMAlloc_t *m_psNext;
};

typedef struct sMMFreeRegion sMMFreeRegion_t;
struct sMMFreeRegion {
  uint32 m_nSize;
  sMMFreeRegion_t *m_psNext;
};

typedef struct {
  void *m_pHeap;
  uint32 m_nPages;

  sMMAlloc_t *m_psAllocHead;
  sMMFreeRegion_t *m_psFreeHead;
}
sMMAllocator_t;

void MM_Init ( sMBInfo_t * psMBInfo );
void MM_Lock ( void );
void MM_Unlock ( void );

/* Core MM routines for handling pages of memory */
void MM_Page_Map ( void *pPhysicalAddr, void *pLogicalAddr, uint32 nPageCount, bool bUser, bool bRW );
void MM_Page_MapZeroFill ( void *pLogicalAddr, uint32 nPageCount, bool bUser, bool bRW );
void MM_Page_Unmap ( void *pLogicalAddr, uint32 nPageCount );
void MM_Page_Allocate ( void *pLogicalAddr, bool bZeroFill );
void MM_Page_Free ( void *pLogicalAddr, uint32 nPageCount );
void MM_Page_AddFree ( void *pPhysicalAddr );
uint32 MM_GetPTAddress ( void *pLogicalAddr );
uint32 MM_GetPTValue ( void *pLogicalAddr );

/* Allocator */
void *MM_KAlloc ( uint32 nSize );
void MM_KFree ( void *pAllocation );
void *MM_KExpand ( void *pAllocation, uint32 nAdditionalSize );
void MM_AllocInit ( sMMAllocator_t * psAllocator, void *pAddress,
                    uint32 nPages );
void *MM_Alloc ( uint32 nSize, sMMAllocator_t * psAllocator );
void *MM_Expand ( void *pAllocation, uint32 nAdditionalSize,
                  sMMAllocator_t * psAllocator );
void MM_Free ( void *pAllocation, sMMAllocator_t * psAllocator );
void MM_Coalesce ( sMMAllocator_t * psAllocator, sMMFreeRegion_t *psFree );

/* Process pages */
void MM_ZeroPageStruct( sProcessPages_t *psProcessPages );

#endif /* __MM_H__ */
