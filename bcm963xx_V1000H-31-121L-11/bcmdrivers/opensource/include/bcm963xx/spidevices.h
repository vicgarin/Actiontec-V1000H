/*
    Copyright 2000-2011 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
*/

#ifndef _SPIDEVICES_H
#define _SPIDEVICES_H

int kerSysBcmSpiSlaveInit( void );
int kerSysBcmSpiSlaveRead(unsigned long addr, unsigned long * data, unsigned long len);
unsigned long kerSysBcmSpiSlaveReadReg32(unsigned long addr);
int kerSysBcmSpiSlaveWrite(unsigned long addr, unsigned long data, unsigned long len);
void kerSysBcmSpiSlaveWriteReg32(unsigned long addr, unsigned long data);
int kerSysBcmSpiSlaveWriteBuf(unsigned long addr, unsigned long *data, unsigned long len, unsigned int unitSize);

#endif
