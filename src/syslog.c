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

extern bool g_bQuietMode;

#define SYSLOG_BUFFER 1024

// Lock
sLock_t l_sSysLogLock;

void Syslog_LockInit() {
  Lock_Init( &l_sSysLogLock );
}

void Syslog_Lock() {
  Lock_Lock( &l_sSysLogLock );
}

void Syslog_Unlock() {
  Lock_Unlock( &l_sSysLogLock );
}

void Syslog_Init() {

  // Initialise lock
  Syslog_LockInit();

}

void Syslog_Entry( char * pszModule, const char * pszFormat, ... ) {

  // If quiet mode enabled don't display to screen
  if ( !g_bQuietMode ) {

    // Display module (if wanted)
    if ( pszModule != NULL ) {
      TextModeVideo_ChangeColour( TEXTCOLOUR_LBLUE );
      TextModeVideo_WriteString( pszModule );

      // Pad with spaces if necessary
      uint32 i;
      for ( i = 0; i < 12 - strlen( pszModule ); i++ ) {
        TextModeVideo_WriteChar( ' ' );
      }

      // Display a separator
      TextModeVideo_ChangeColour( TEXTCOLOUR_LPURPLE );
      TextModeVideo_WriteChar( ' ' );
      TextModeVideo_WriteChar( '-' );
      TextModeVideo_WriteChar( ' ' );
    }

    // Use printf to generate buffer
    char pszConsoleBuffer[ SYSLOG_BUFFER ];
    va_list args;
    va_start( args, pszFormat );
    vsnprintf( ( char * ) & pszConsoleBuffer, SYSLOG_BUFFER, pszFormat, args );
    va_end( args );
    TextModeVideo_ChangeColour( TEXTCOLOUR_WHITE );
    TextModeVideo_WriteString( ( char * ) & pszConsoleBuffer );
  }
}
