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

#ifndef __IPC_H__
#define __IPC_H__

/* Error messages */
enum {
  IPC_SUCCESS,
  IPC_PORTALREADYEXISTS,
  IPC_PORTDOESNTEXIST,
  IPC_ALREADYEDITINGMESSAGE
};

/* Message port struct */
typedef struct {
  uint32 m_nMessagesWaiting;	   	   // Number of messages in queue
  void *m_psTail;	   	   // Last event in list (more added on here)
  void *m_psHead;	   	   // First event in list (taken off here when processed)
  bool m_bProcessingMessage;		   // Are we currently processing message?
  void *m_psMessage;		   // Current message being constructed
}
sMessagePort_t;

/* Message struct */
typedef struct {
  uint32 m_nMessageCode;	   	   // Message code
  void *m_pData;			   // Serialised data (i.e. parameters)
}
sMessage_t;

void IPC_Init( void );
KStatus IPC_CreatePort( void );
KStatus IPC_GetMessageQueueLength( uint32 *pnLength );
KStatus IPC_CreateMessage( uint32 nMessageCode, void *pData );

#endif /* __IPC_H__ */
