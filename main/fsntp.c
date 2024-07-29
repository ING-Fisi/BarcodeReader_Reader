/*
 */


/* --------------------- INCLUDES ------------------------ *
 * ------------------------------------------------------- */
// ESP_IDF
#include "freertos/FreeRTOS.h"
#include "esp_sntp.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "sdkconfig.h"
// Custom
#include "fsntp.h"
#include "utility.h"

/* --------------------- DEFINES ------------------------- *
 * ------------------------------------------------------- */

/* --------------------- VARIABLES ----------------------- *
 * ------------------------------------------------------- */
static const char* TAG = "XSNTP";
static uint8_t time_sync_flag = 0;

/* --------------------- FUNCTIONS ----------------------- *
 * ------------------------------------------------------- */
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGD(TAG, "Time synchronization event.");
    time_sync_flag = 1;
}

static void initialize_sntp( char* sntp_server) {
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	if ( sntp_server == NULL ) {
	    sntp_server = DEFAULT_SNTP_SERVER;
	}
    ESP_LOGI(TAG, "Initialize SNTP server: %s.", sntp_server);
	sntp_setservername(0, sntp_server);
	sntp_init();
}

int sntp_init_time( char* sntp_server, int tmout_s) {
    time_t now = 0;
    struct tm timeinfo = {0};
    set_curtimestamp(0);			// Reset internal timestamp.
    long t = get_curtimestamp();
    int tmt = DEFAULT_SNTP_TMOUT;
    if ( tmout_s != 0 ) {
    	tmt = tmout_s;
    }
    initialize_sntp(sntp_server);
    // wait for time to be set
    ESP_LOGI(TAG, "Waiting for system time to be set..");
    while ( timeinfo.tm_year < (2016 - 1900) ) {
    	esp_task_wdt_reset();
        ESP_LOGD(TAG,"flag: %d, time: %ld, elapsed: %ld", time_sync_flag, get_curtimestamp(), (get_curtimestamp()-t) );
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    	if ( (time_sync_flag == 0) && ( (get_curtimestamp()-t) > tmt) ) {
    		ESP_LOGW(TAG,"Obtain time function return timeout.");
        	sntp_stop();
    		return -2;
    	}
    }
    ESP_LOGI(TAG, "..time is set: %ld.", get_curtimestamp());
    return 0;
}
