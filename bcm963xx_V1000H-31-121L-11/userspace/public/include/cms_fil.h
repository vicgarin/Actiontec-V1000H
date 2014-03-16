/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
 ************************************************************************/


#ifndef __CMS_FIL_H__
#define __CMS_FIL_H__

#define KILOBYTE 1024

/*!\file cms_fil.h
 * \brief Header file for file and directory utilities functions.
 */


/** Return true if the filename exists.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return TRUE if the specified file exists.
 */
UBOOL8 cmsFil_isFilePresent(const char *filename);



/** Return the size of the file.
 *
 * @param filename (IN) full pathname to the file.
 *
 * @return -1 if the file does not exist, otherwise, the file size.
 */
SINT32 cmsFil_getSize(const char *filename);



/** Copy the contents of the file to the specified buffer.
 *
 * @param filename (IN) full pathname to the file.
 * @param buf     (OUT) buffer that will hold contents of the file.
 * @bufSize    (IN/OUT) On input, the size of the buffer, on output
 *                      the actual number of bytes that was copied
 *                      into the buffer.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize);


/** Write the specified string into the specified proc file.
 *
 * @param procFile (IN) Name of the proc file.
 * @param s        (IN) String to write into the proc file.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_writeToProc(const char *procFilename, const char *s);


/** Write the buffer into the specified file.
 *
 * @param filename (IN) Name of the file.
 * @param buf      (IN) Buffer to write.
 * @param bufLen   (IN) Length of buffer.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_writeBufferToFile(const char *filename, const UINT8 *buf, UINT32 bufLen);


/** Remove specified directory.  If there are any files or sub-dirs in the
 * directory, they will be silently removed.
 *
 * @param dirname (IN) Name of the directory to remove.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_removeDir(const char *dirname);


/** Create the specified directory.
 *
 * @param dirname (IN) Name of the directory to create.
 *
 * @return CmsRet enum.
 */
CmsRet cmsFil_makeDir(const char *dirname);


/** convert the size into KiloBytes 
 *
 * @param nblks (IN) number of blocks.
 * @param blkSize (IN) size of each block in bytes.
 *
 * @return UINT32 (size in Kilo Bytes)
 */

UINT32 cmsFil_scaleSizetoKB(long nblks, long blkSize);
#endif /* __CMS_FIL_H__ */
