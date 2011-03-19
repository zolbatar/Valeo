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

#define TMV_ADDR 0xB8000
#define TMV_PAGES 0x8
#define COLUMNS 80
#define ROWS 25

#define ADDRESSREGISTER 0x3d4
#define DATAREGISTER 0x3d5

// Keep track of screen position
volatile uchar *g_pVideo;
uint32 nX;
uint32 nY;
uint32 nPos;
uchar nAttr = TEXTCOLOUR_WHITE;

void TextModeVideo_Init ()
{

    // Set memory address
    g_pVideo = ( uchar * ) TMV_ADDR;

    // Change cursor shape
    outb ( ADDRESSREGISTER, ( uchar ) 0x9 );
    uchar nCursorSizeMax = inb ( DATAREGISTER ) & 0x1f;
    outb ( ADDRESSREGISTER, ( uchar ) 0xa );
    outb ( DATAREGISTER, ( uchar ) nCursorSizeMax >> 1 );
    outb ( ADDRESSREGISTER, ( uchar ) 0xb );
    outb ( DATAREGISTER, ( uchar ) nCursorSizeMax );

    // And clear console
    TextModeVideo_Cls ();
}

void TextModeVideo_PostMMInit ()
{

    // Move video memory from 0xB8000 to physical memory map area
    MM_Lock ();
    MM_Page_Map ( ( void * ) TMV_ADDR, ( void * ) MM_KERNEL_PMM, TMV_PAGES, false, true );
    g_pVideo = ( unsigned char * ) MM_KERNEL_PMM;
    //  g_sKernel.m_pMemory_PMMEnd += TMV_PAGES << MM_PAGE_OFFSETBITS;
    MM_Unlock ();
}

void TextModeVideo_MoveCursor ()
{

    // Change cursor position on screen
    uint32 nPosition = ( nY * 80 ) + nX;
    outb ( ADDRESSREGISTER, ( uchar ) 0xf );
    outb ( DATAREGISTER, ( uchar ) nPosition & 0xFF );
    outb ( ADDRESSREGISTER, ( uchar ) 0xe );
    outb ( DATAREGISTER, ( uchar ) ( nPosition >> 8 ) & 0xFF );
}

void TextModeVideo_Cls ()
{

    // Clear video memory
    memset ( ( void * ) g_pVideo, 0, COLUMNS * ROWS * 2 );

    // Initialise cursor variables
    nX = 0;
    nY = 0;
    nPos = 0;
    TextModeVideo_MoveCursor ();
}

void TextModeVideo_ChangeColour ( uchar nColour )
{
    nAttr = nColour;
}

void TextModeVideo_WriteChar ( uchar c )
{

    // Write character at cursor and scroll etc. as needed
    if ( c == '\n' || c == '\r' ) {
    newline:
        nX = 0;
        nY++;
        nPos = ( nY * 160 );
        if ( nY == ROWS ) {

            // Scroll 1 line up
            memcpy ( ( void * ) g_pVideo,
                     ( void * ) g_pVideo + ( COLUMNS * 2 ),
                     ( ( ROWS - 1 ) * COLUMNS * 2 ) );

            // Clear new lines
            memset ( ( void * ) g_pVideo +
                     ( ( ROWS - 1 ) * COLUMNS * 2 ), 0,
                     ( 1 * COLUMNS * 2 ) );
            nY--;
            nPos -= 160;
        }
        TextModeVideo_MoveCursor ();
        return ;
    }

    // Set character and attribute
    g_pVideo[ nPos ] = c & 0xFF;
    g_pVideo[ nPos + 1 ] = nAttr;

    nX++;
    nPos += 2;
    if ( nX >= COLUMNS ) {
        goto newline;
    }
}

void TextModeVideo_DeleteChar ()
{

    // Delete character at cursor
    nX--;
    nPos -= 2;
    *( g_pVideo + nPos ) = 0;
    *( g_pVideo + nPos + 1 ) = nAttr;
}

void TextModeVideo_WriteString ( char *pszString )
{

    // Output to screen
    int i = 0;
    while ( pszString[ i ] != 0 ) {
        TextModeVideo_WriteChar ( pszString[ i++ ] );
    }
}
