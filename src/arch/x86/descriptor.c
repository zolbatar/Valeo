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

// Descriptors
sIDT_t *l_sIDT;
sGDT_t *l_sGDT;

void Descriptor_Init ( sIDT_t * sIDT, sGDT_t * sGDT )
{

    // Set IDT/GDT locations
    l_sIDT = sIDT;
    l_sGDT = sGDT;
}

void Descriptor_SetInterruptGate ( uint32 nInterrupt, void *pHandler )
{

    l_sIDT[ nInterrupt ].m_nSelector = KERNELCS;
    l_sIDT[ nInterrupt ].m_nBaseLow = ( ( uint32 ) pHandler ) & 0xFFFF;
    l_sIDT[ nInterrupt ].m_nBaseHigh = ( ( uint32 ) pHandler ) >> 16;
    l_sIDT[ nInterrupt ].m_nFlags = 0x8E00;
}

void Descriptor_SetTaskGate ( uint32 nInterrupt, void *pHandler )
{

    l_sIDT[ nInterrupt ].m_nSelector = KERNELCS;
    l_sIDT[ nInterrupt ].m_nBaseLow = ( ( uint32 ) pHandler ) & 0xFFFF;
    l_sIDT[ nInterrupt ].m_nBaseHigh = ( ( uint32 ) pHandler ) >> 16;
    l_sIDT[ nInterrupt ].m_nFlags = 0x8500;
}

void Descriptor_SetCallGate ( uint32 nInterrupt, void *pHandler )
{

    l_sIDT[ nInterrupt ].m_nSelector = KERNELCS;
    l_sIDT[ nInterrupt ].m_nBaseLow = ( ( uint32 ) pHandler ) & 0xFFFF;
    l_sIDT[ nInterrupt ].m_nBaseHigh = ( ( uint32 ) pHandler ) >> 16;
    l_sIDT[ nInterrupt ].m_nFlags = 0x8C00;
}

void Descriptor_SetBase ( uint16 nDescriptor, uint32 nBase )
{

    nDescriptor >>= 3;
    l_sGDT[ nDescriptor ].m_nBaseLow = nBase & 0xffff;
    l_sGDT[ nDescriptor ].m_nBaseMiddle = ( nBase >> 16 ) & 0xff;
    l_sGDT[ nDescriptor ].m_nBaseHigh = ( nBase >> 24 ) & 0xff;
}

void Descriptor_SetLimit ( uint16 nDescriptor, uint32 nLimit )
{

    nDescriptor >>= 3;
    l_sGDT[ nDescriptor ].m_nLimitHigh = 0x40;

    // See if we need 4K granularity
    if ( nLimit > 0x000fffff ) {
        l_sGDT[ nDescriptor ].m_nLimitHigh |= 0x80;
        nLimit >>= 12;
    }

    l_sGDT[ nDescriptor ].m_nLimitLow = nLimit & 0xffff;
    l_sGDT[ nDescriptor ].m_nLimitHigh |= ( nLimit >> 16 ) & 0x0f;
}

void Descriptor_SetAccess ( uint16 nDescriptor, uint8 nAccess )
{

    nDescriptor >>= 3;
    l_sGDT[ nDescriptor ].m_nAttributes = nAccess;
}
