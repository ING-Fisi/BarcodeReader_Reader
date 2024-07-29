/*
 * x-sntp.h
 *
 *  Created on: Mar 4, 2020
 *      Author: X-Phase s.r.l., Leonardo Volpi
 */

#ifndef MAIN_X_SNTP_H_
#define MAIN_X_SNTP_H_

/* --------------------- INCLUDES ------------------------ *
 * ------------------------------------------------------- */

/* --------------------- DEFINES ------------------------- *
 * ------------------------------------------------------- */
// Functionality
#define DEFAULT_SNTP_TMOUT 30
// SNTP servers
#define UNIFI_SNTP "ntp.unifi.it"
#define GOOGLE_SNTP "time.google.com"
#define POOL_PRJCT_SNTP "pool.ntp.org"
#define DEFAULT_SNTP_SERVER GOOGLE_SNTP // Select one of the previous servers



/* --------------------- VARIABLES ----------------------- *
 * ------------------------------------------------------- */

/* --------------------- FUNCTIONS ----------------------- *
 * ------------------------------------------------------- */
/* Function that initialize SNTP service and recover time	*
 * from specified SNTP server. Blocks till time has been	*
 * recovered or specified timeout occurs.					*
 *															*
 * @param:													*
 * 	sntp_server	: SNTP server URL string, if NULL			*
 * 						  default will be selected			*
 * 	tmout_ms	: Timeout in seconds, if NULL				*
 * 						  default will be selected			*
 *															*
 * @return result		: 0, -1 or -2 (timeout)				*/
int sntp_init_time( char* sntp_server, int tmout_s);



#endif /* MAIN_X_SNTP_H_ */
