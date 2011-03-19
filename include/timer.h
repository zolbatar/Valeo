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

#ifndef __TIMER_H__
#define __TIMER_H__

typedef KStatus TimerHandler( sCallStack_t *psStack );

typedef struct sTimer sTimer_t;
struct sTimer {
    uint32 m_nTimeOut;
    uint32 m_nTimeOutLeft;
    TimerHandler *m_pHandler;
    sTimer_t *m_psPrevious;
};

void Timer_Init( void );
void Timer_AttachTimerHandler( uint32 nTimeOut, TimerHandler *pHandler );

#endif /* __TIMER_H__ */
