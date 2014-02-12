/*----------------------------------------------------------------------------------------------------*
 *                                                                                                    *
 *  T z C L O C K . C                                                                                 *
 *  =================                                                                                 *
 *                                                                                                    *
 *  TzClock developed by Chris Knight based on glock by Eric L. Smith.                                * 
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
 
#ifndef INCLUDE_TZC_CONFIG_H
#define INCLUDE_TZC_CONFIG_H
 
/*
 * config.c
 */

int configLoad (const char *configFile);
int configSave (const char *configFile);
void configFree ();
int configSetValue (char *configName, char *configValue);
int configSetIntValue (char *configName, int configValue);
int configSetBoolValue (char *configName, bool configValue);
int configGetValue (char *configName, char *value, int maxLen);
int configGetIntValue (const char *configName, int *value);
int configGetBoolValue (char *configName, bool *configValue);

#endif
