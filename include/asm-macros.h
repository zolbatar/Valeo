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

#ifndef __ASMMACROS_H__
#define __ASMMACROS_H__

#define	ENTER_INTERRUPT	        \
	cli;			\
	cld;			\
	push  %ds;		\
	push  %es;		\
	push  %fs;		\
	push  %gs;		\
	pushl %eax;             \
	pushl %ebx;             \
	pushl %ecx;             \
	pushl %edx;             \
	pushl %ebp;             \
	pushl %esi;             \
	pushl %edi;             \
	pushl %ss;		\
	pushl %esp;		\
	pushl %esp;

#define EXIT_INTERRUPT 		\
	addl $4, %esp;		\
	lss (%esp), %esp;	\
	addl $4, %esp;		\
	popl %edi;              \
	popl %esi;              \
	popl %ebp;              \
	popl %edx;              \
	popl %ecx;              \
	popl %ebx;              \
	popl %eax;              \
	pop   %gs;	        \
	pop   %fs;	        \
	pop   %es;	        \
	pop   %ds;	        \
	sti;  			\
	iret

#define EXIT_SYSCALL 		\
	addl $4, %esp;		\
	lss (%esp), %esp;	\
	addl $4, %esp;		\
	popl %edi;              \
	popl %esi;              \
	popl %ebp;              \
	popl %edx;              \
	popl %ecx;              \
	popl %ebx;              \
	addl $4, %esp;		\
	pop   %gs;	        \
	pop   %fs;	        \
	pop   %es;	        \
	pop   %ds;	        \
	sti;  			\
	iret

#define	ENTEREXCEPTION 	\
	cld;			\
	push  %ds;		\
	push  %es;		\
	push  %fs;		\
	push  %gs;              \
	pushl %eax;             \
	pushl %ebx;             \
	pushl %ecx;             \
	pushl %edx;             \
	pushl %ebp;             \
	pushl %esi;             \
	pushl %edi;             \
	pushl %esp;

#define EXITEXCEPTION_WITHERROR\
	addl $4, %esp;		\
	popl %edi;              \
	popl %esi;              \
	popl %ebp;              \
	popl %edx;              \
	popl %ecx;              \
	popl %ebx;              \
	popl %eax;              \
	pop   %gs;	        \
	pop   %fs;	        \
	pop   %es;	        \
	pop   %ds;	        \
	addl  $4, %esp;		\
	iret

#define EXITEXCEPTION		\
	addl $4, %esp;		\
	popl %edi;              \
	popl %esi;              \
	popl %ebp;              \
	popl %edx;              \
	popl %ecx;              \
	popl %ebx;              \
	popl %eax;              \
	pop   %gs;	        \
	pop   %fs;	        \
	pop   %es;	        \
	pop   %ds;	        \
	iret

#endif /* __ASMMACROS_H__ */
