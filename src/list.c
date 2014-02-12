/**********************************************************************************************************************
 *                                                                                                                    *
 *  L I S T . C                                                                                                       *
 *  ===========                                                                                                       *
 *                                                                                                                    *
 *  This is free software; you can redistribute it and/or modify it under the terms of the GNU General Public         *
 *  License version 2 as published by the Free Software Foundation.  Note that I am not granting permission to        *
 *  redistribute or modify this under the terms of any later version of the General Public License.                   *
 *                                                                                                                    *
 *  This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the                *
 *  impliedwarranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   *
 *  more details.                                                                                                     *
 *                                                                                                                    *
 *  You should have received a copy of the GNU General Public License along with this program (in the file            *
 *  "COPYING"); if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,   *
 *  USA.                                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @file
 *  @brief .
 *  @version $Id: list.c 1811 2013-12-20 08:29:07Z ukchkn $
 */
#include <stdlib.h>
#include <string.h>
#include "list.h"

#ifdef MULTI_THREAD
	#ifdef WIN32
		#include <windows.h>
	#endif
#endif

/*----------------------------------------------------------------------------------------------------*
 *                                                                                                    *
 * Structure to hold an item on the queue                                                             *
 *                                                                                                    *
 *----------------------------------------------------------------------------------------------------*/
typedef struct _queueItem
{
	void *myNextPtr;
	void *myPrevPtr;
	void *myData;
}
QUEUE_ITEM;

/*----------------------------------------------------------------------------------------------------*
 *                                                                                                    *
 * Structure to hold the queue header                                                                 *
 *                                                                                                    *
 *----------------------------------------------------------------------------------------------------*/
typedef struct _queueHeader
{
	QUEUE_ITEM *firstInQueue;
	QUEUE_ITEM *lastInQueue;
	QUEUE_ITEM *currentItem;
	unsigned long itemCount;
	unsigned long freeData;

#ifdef MULTI_THREAD
#ifdef WIN32

	HANDLE queueMutex;

#else

	pthread_mutex_t queueMutex;

#endif
#endif
}
QUEUE_HEADER;

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  L O C K                                                                                                *
 *  ==================                                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param myQueue .
 *  @result .
 */
static void queueLock (QUEUE_HEADER *myQueue)
{
#ifdef MULTI_THREAD
#ifdef WIN32

	WaitForSingleObject (myQueue -> queueMutex, INFINITE);

#else

	pthread_mutex_lock(&myQueue -> queueMutex);

#endif
#endif
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  U N  L O C K                                                                                           *
 *  =======================                                                                                           *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param myQueue .
 *  @result .
 */
static void queueUnLock (QUEUE_HEADER *myQueue)
{
#ifdef MULTI_THREAD
#ifdef WIN32

	ReleaseMutex (myQueue -> queueMutex);

#else

	pthread_mutex_unlock(&myQueue -> queueMutex);

#endif
#endif
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  C R E A T E                                                                                            *
 *  ======================                                                                                            *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @result .
 */
void *queueCreate ()
{
	QUEUE_HEADER *newQueue;

	if ((newQueue = malloc (sizeof (QUEUE_HEADER))) == NULL)
		return NULL;

	newQueue -> firstInQueue = NULL;
	newQueue -> lastInQueue = NULL;
	newQueue -> currentItem = NULL;
	newQueue -> itemCount = 0;
	newQueue -> freeData = 0;

#ifdef MULTI_THREAD
#ifdef WIN32

	newQueue -> queueMutex = CreateMutex (NULL, FALSE, "Queue");

#else

	pthread_mutex_init(&newQueue -> queueMutex, NULL);

#endif
#endif
	return newQueue;
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  D E L E T E                                                                                            *
 *  ======================                                                                                            *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @result .
 */
void queueDelete (void *queueHandle)
{
#ifdef MULTI_THREAD

	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;

#ifdef WIN32

	CloseHandle (myQueue -> queueMutex);

#else

	pthread_mutex_destroy(&myQueue -> queueMutex);

#endif
#endif

	if (queueHandle)
		free (queueHandle);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  G E T                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @result .
 */
void *queueGet (void *queueHandle)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	QUEUE_ITEM *oldQueueItem;
	void *retn = NULL;

	queueLock (myQueue);
	if (myQueue -> firstInQueue)
	{
		oldQueueItem = myQueue -> firstInQueue;
		retn = oldQueueItem -> myData;
		myQueue -> firstInQueue = oldQueueItem -> myNextPtr;
		free (oldQueueItem);

		if (myQueue -> firstInQueue)
			myQueue -> firstInQueue -> myPrevPtr = NULL;
		else
			myQueue -> lastInQueue = NULL;
			
		myQueue -> itemCount --;
	}
	queueUnLock (myQueue);
	return retn;
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  P U T                                                                                                  *
 *  ================                                                                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @param putData .
 *  @result .
 */
void queuePut (void *queueHandle, void *putData)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	QUEUE_ITEM *newQueueItem;

	if ((newQueueItem = malloc (sizeof (QUEUE_ITEM))) == NULL)
		return;

	newQueueItem -> myNextPtr = newQueueItem -> myPrevPtr = NULL;
	newQueueItem -> myData = putData;

	queueLock (myQueue);
	if (myQueue -> lastInQueue)
	{
		myQueue -> lastInQueue -> myNextPtr = newQueueItem;
		newQueueItem -> myPrevPtr = myQueue -> lastInQueue;
		myQueue -> lastInQueue = newQueueItem;
	}
	else
		myQueue -> firstInQueue = myQueue -> lastInQueue = newQueueItem;

	myQueue -> itemCount ++;
	queueUnLock (myQueue);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  P U S H                                                                                                *
 *  ==================                                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @param putData .
 *  @result .
 */
void queuePush (void *queueHandle, void *putData)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	QUEUE_ITEM *newQueueItem;

	if ((newQueueItem = malloc (sizeof (QUEUE_ITEM))) == NULL)
		return;

	newQueueItem -> myNextPtr = newQueueItem -> myPrevPtr = NULL;
	newQueueItem -> myData = putData;

	queueLock (myQueue);
	if (myQueue -> firstInQueue)
	{
		myQueue -> firstInQueue -> myPrevPtr = newQueueItem;
		newQueueItem -> myNextPtr = myQueue -> firstInQueue;
		myQueue -> firstInQueue = newQueueItem;
	}
	else
		myQueue -> firstInQueue = myQueue -> lastInQueue = newQueueItem;

	myQueue -> itemCount ++;
	queueUnLock (myQueue);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  P U T  S O R T                                                                                         *
 *  =========================                                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @param putData .
 *  @param item2 .
 *  @result .
 */
void queuePutSort (void *queueHandle, void *putData, 
		int(*Compare)(void *item1, void *item2))
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	QUEUE_ITEM *newQueueItem;

	if ((newQueueItem = malloc (sizeof (QUEUE_ITEM))) == NULL)
		return;

	newQueueItem -> myNextPtr = newQueueItem -> myPrevPtr = NULL;
	newQueueItem -> myData = putData;

	queueLock (myQueue);
	if (myQueue -> firstInQueue)
	{
		QUEUE_ITEM *currentItem = myQueue -> firstInQueue;
	
		while (currentItem)
		{
			int comp = Compare (putData, currentItem -> myData);
			
			if (comp < 0)
			{
				newQueueItem -> myPrevPtr = currentItem -> myPrevPtr;
				newQueueItem -> myNextPtr = currentItem;
				
				if (newQueueItem -> myPrevPtr)
				{
					QUEUE_ITEM *tempItem = newQueueItem -> myPrevPtr;
					tempItem -> myNextPtr = newQueueItem;
				}
				else
					myQueue -> firstInQueue = newQueueItem;

				currentItem -> myPrevPtr = newQueueItem;
				break;
			}
			currentItem = currentItem -> myNextPtr;
		}
		if (!currentItem)
		{
			myQueue -> lastInQueue -> myNextPtr = newQueueItem;
			newQueueItem -> myPrevPtr = myQueue -> lastInQueue;
			myQueue -> lastInQueue = newQueueItem;
		}
	}
	else
		myQueue -> firstInQueue = myQueue -> lastInQueue = newQueueItem;

	myQueue -> itemCount ++;
	queueUnLock (myQueue);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  R E A D                                                                                                *
 *  ==================                                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @param item .
 *  @result .
 */
void *queueRead (void *queueHandle, int item)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	QUEUE_ITEM *currentItem = myQueue -> firstInQueue;
	void *retn = NULL;

	queueLock (myQueue);
	while (currentItem && item > 0)
	{
		currentItem = currentItem -> myNextPtr;
		item --;
	}
	if (currentItem)
		retn = currentItem -> myData;

	queueUnLock (myQueue);
	return retn;
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  S E T  F R E E  D A T A                                                                                *
 *  ==================================                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @param setData .
 *  @result .
 */
void queueSetFreeData (void *queueHandle, unsigned long setData)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	queueLock (myQueue);
	myQueue -> freeData = setData;
	queueUnLock (myQueue);
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  G E T  F R E E  D A T A                                                                                *
 *  ==================================                                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @result .
 */
unsigned long queueGetFreeData (void *queueHandle)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	unsigned long retn;
	
	queueLock (myQueue);
	retn = myQueue -> freeData;
	queueUnLock (myQueue);
	return retn;
}

/**********************************************************************************************************************
 *                                                                                                                    *
 *  Q U E U E  G E T  I T E M  C O U N T                                                                              *
 *  ====================================                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/
/**
 *  @brief .
 *  @param queueHandle .
 *  @result .
 */
unsigned long queueGetItemCount (void *queueHandle)
{
	QUEUE_HEADER *myQueue = (QUEUE_HEADER *)queueHandle;
	unsigned long retn;
	
	queueLock (myQueue);
	retn = myQueue -> itemCount;
	queueUnLock (myQueue);
	return retn;
}
