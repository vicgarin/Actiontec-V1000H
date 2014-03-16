/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
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

#ifndef __CMS_XML_H__
#define __CMS_XML_H__



/*!\file cms_xml.h
 * \brief Header file for XML string escaping/unescaping functions.
 *
 * These functions allow callers to escape strings that contain
 * reserved XML characters, < > % &, and unescape them again.
 *
 */


/** Escape any reserved XML characters in the given string.
 * 
 * @param string        (IN)  Input string which may contain reserved XML characters.
 * @param escapedString (OUT) This function will allocate a buffer and put the
 *                            escaped string in it.  The caller is
 *                            responsible for freeing this buffer.
 *
 * @return CmsRet enum.
 */ 
CmsRet cmsXml_escapeString(const char *string, char **escapedString);


/** Convert a string with escaped XML characters back into normal string.
 * 
 * @param escapedString (IN)  Input string which may contain escaped character sequences.
 * @param string        (OUT) This function will allocate a buffer and put
 *                            the unescaped string into the buffer.  The caller
 *                            is responsible for freeing the buffer.
 * @return CmsRet enum.
 */
CmsRet cmsXml_unescapeString(const char *escapedString, char **string);



#endif  /* __CMS_HEXBINARY_H__ */
