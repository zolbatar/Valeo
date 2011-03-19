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

// Lock
sLock_t sResourceLock;

void Resource_LockInit() {
  Lock_Init( &sResourceLock );
}

void Resource_Lock() {
  Lock_Lock( &sResourceLock );
}

void Resource_Unlock() {
  Lock_Unlock( &sResourceLock );
}

void Resource_Init() {

  // Initialise locking
  Resource_LockInit();

  Syslog_Entry ( "RESOURCE", "Resource manager initialised\n");
}

void Resource_Request(uint32 nResourceType, uint32 nValue) {
}
