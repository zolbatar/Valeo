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

#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

typedef struct {
    uint32 m_nMagic;
    uint32 m_nFlags;
    uint32 m_nChecksum;
    uint32 m_nHeaderAddr;
    uint32 m_nLoadAddr;
    uint32 m_nLoadEndAddr;
    uint32 m_nBSSEndAddr;
    uint32 m_nEntryAddr;
}
sMBHeader_t;

/* The symbol table for a.out */
typedef struct {
    uint32 m_nTabSize;
    uint32 m_nStrSize;
    uint32 m_nAddr;
    uint32 m_nReserved;
}
sMBAOUTHeader_t;

/* The section header table for ELF */
typedef struct {
    uint32 m_nNum;
    uint32 m_nSize;
    uint32 m_nAddr;
    uint32 m_nShndx;
}
sMBELFHeader_t;

/* The Multiboot information */
typedef struct {
    uint32 m_nFlags;
    uint32 m_nMemLower;
    uint32 m_nMemUpper;
    uint32 m_nBootDevice;
    uint32 m_nCmdLine;
    uint32 m_nModsCount;
    uint32 m_nModsAddr;
    union
    {
        sMBAOUTHeader_t m_nAoutSym;
        sMBELFHeader_t m_nElfSec;
    } u;
    uint32 m_nMMAPLength;
    uint32 m_nMMAPAddr;
}
sMBInfo_t;

/* The module structure */
typedef struct {
    uint32 m_nStart;
    uint32 m_nEnd;
    uint32 m_nString;
    uint32 m_nReserved;
}
sMBModule_t;

/* The memory map */
typedef struct {
    uint32 m_nSize;
    uint32 m_nBaseAddrLow;
    uint32 m_nBaseAddrHigh;
    uint32 m_nLengthLow;
    uint32 m_nLengthHigh;
    uint32 m_nType;
}
sMBMemoryMap_t;

#endif /* __MULTIBOOT_H__ */

