/*----------------------------------------------------------------------------------------------------*
 *                                                                                                    *
 *  L I S T . H                                                                                       *
 *  ===========                                                                                       *
 *                                                                                                    *
 *----------------------------------------------------------------------------------------------------*
 *                                                                                                    *
 *  TzClock is free software; you can redistribute it and/or modify it under the terms of the GNU     *
 *  General Public License version 2 as published by the Free Software Foundation.  Note that I       *
 *  am not granting permission to redistribute or modify TzClock under the terms of any later         *
 *  version of the General Public License.                                                            *
 *                                                                                                    *
 *  TzClock is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without      *
 *  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                                                      *
 *                                                                                                    *
 *  You should have received a copy of the GNU General Public License along with this program (in     * 
 *  the file "COPYING"); if not, write to the Free Software Foundation, Inc.,                         *
 *  59 Temple Place - Suite 330, Boston, MA 02111, USA.                                               *
 *                                                                                                    *
 *----------------------------------------------------------------------------------------------------*/
/**
 *  @file
 *  @brief .
 *  @version $Id: list.h 905 2010-01-21 10:26:27Z ukchkn $
 */

void *queueCreate (void);
void  queueDelete (void *queueHandle);
void *queueGet (void *queueHandle);
void  queuePut (void *queueHandle, void *putData);
void  queuePutSort (void *queueHandle, void *putData, 
                        int(*Compare)(void *item1, void *item2));
void  queuePush (void *queueHandle, void *putData);
void *queueRead (void *queueHandle, int item);
void queueSetFreeData (void *queueHandle, unsigned long setData);
unsigned long queueGetFreeData (void *queueHandle);
unsigned long queueGetItemCount (void *queueHandle);

