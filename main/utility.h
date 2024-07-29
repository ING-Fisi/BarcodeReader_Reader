/*
 * common.h
 *
 *  Created on: July 26, 2019
 *      Author: X-Phase s.r.l., Leonardo Volpi
 */

#ifndef MAIN_X_COMMON_H_
#define MAIN_X_COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

/* ----- Common Defines ----- */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

#define MYMACSTR "%02x%02x%02x%02x%02x%02x"

#define MSEC (1000) 			// based on esp_timer_get_time() that return us from boot
#define SEC (1000*MSEC)
#define MINUTE (60*SEC)
#define  T_US esp_timer_get_time()

#define VAL2INT(buf,idest,val,size) {\
	memcpy(buf,val,size);\
	buf[size] = 0;\
	idest = atoi(buf);\
}

/* ----- Function Prototypes ----- */
void print_chip_info ( void );
void set_curtimestamp( long tmstmp );
long get_curtimestamp( void );
u64 get_curtimestampms( void );
int chck_time_int( long *t_hold, long t_int );
int chck_time_int_ms( long *t_hold, long t_int );
int ts2time( long timestamp, char *buf);
int ts2date( long timestamp, char *buf);
int ts2datetime( long timestamp, char *buf);
long gnsst2ts(char *ptr);
void save_reboot_reason( uint8_t reason );
void get_mac_str(char macstr[]);
int id_from_mac(void);
float rand_int_decimal( u32 interval, u32 decimal);

// GPIO
int set_gpio_output( u32 pin, u8 pup, u8 pdw, u8 intr );
int set_gpio_input( u32 pin, u8 pup, u8 pdw, u8 intr );

#endif /* MAIN_X_COMMON_H_ */
