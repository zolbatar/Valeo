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

#ifndef __X86_DESCRIPTOR_H__
#define __X86_DESCRIPTOR_H__

#define PROCESSOR_NUMINTERRUPTGATES 256

/* IDT structure */
typedef struct
{
	uint16 m_nBaseLow;
	uint16 m_nSelector;
	uint16 m_nFlags;
	uint16 m_nBaseHigh;
}
sIDT_t;

typedef struct
{
	uint32 nLimit;
	void *pBase;
}
sIDTR_t;

/* GDT Structure */
typedef struct
{
	uint16 m_nLimitLow;
	uint16 m_nBaseLow;
	uint8 m_nBaseMiddle;
	uint8 m_nAttributes;
	uint8 m_nLimitHigh;
	uint8 m_nBaseHigh;
}
sGDT_t;

void Descriptor_Init (sIDT_t * sIDT, sGDT_t * sGDT);
void Descriptor_SetInterruptGate (uint32 nInterrupt, void *pHandler);
void Descriptor_SetTaskGate (uint32 nInterrupt, void *pHandler);
void Descriptor_SetCallGate (uint32 nInterrupt, void *pHandler);
void Descriptor_SetBase (uint16 nDescriptor, uint32 nBase);
void Descriptor_SetLimit (uint16 nDescriptor, uint32 nLimit);
void Descriptor_SetAccess (uint16 nDescriptor, uint8 nAccess);

#endif /* __X86_DESCRIPTOR_H__ */
