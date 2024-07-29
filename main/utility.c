/*
*/

#include "utility.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "driver/gpio.h"


/* ------ Global Variables ----- */
static char* TAG = "COMMON"; // log tag
#define VERSION_MAJOR	3
#define VERSION_MINOR	3
#define VERS_CHAR		v
#define STRINGIFY(x)	#x
#define VERSION_STR(A,B,C) 	STRINGIFY(C) STRINGIFY(A) "." STRINGIFY(B)

const char *fw_ver_str;
const char *fw_v = VERSION_STR(VERSION_MAJOR, VERSION_MINOR, VERS_CHAR);

/* ----- Function Prototypes Implementation ----- */
void print_chip_info ( void ) {
	fw_ver_str = fw_v;
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG,"This is ESP32 chip with %d CPU cores, WiFi%s%s",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG,"silicon revision %d", chip_info.revision);

    ESP_LOGI(TAG,"%dMB %s flash", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGI(TAG, "Firmware: %s", fw_ver_str);
}

void set_curtimestamp( long tmstmp ) {
	struct timeval tv;
	tv.tv_sec = tmstmp;
	tv.tv_usec = 0;
	settimeofday( &tv, NULL);
}

long get_curtimestamp( void ) {
	time_t t;
	time(&(t));
    return t;
}

u64 get_curtimestampms( void ) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((u64)(tp.tv_sec) * 1000 + (u64)(tp.tv_usec)/1000);
}

int chck_time_int( long *t_hold, long t_int ) {
	long tmp_hold_t = get_curtimestamp();
	long ret = ((long) (tmp_hold_t - *t_hold) > (long)t_int);
	*t_hold = ret == 1 ? tmp_hold_t : *t_hold;
	return ret;
}

int chck_time_int_ms( long *t_hold, long t_int ) {
	u64 tmp_hold_t = get_curtimestampms();
	u64 ret = ((u64) (tmp_hold_t - *t_hold) > (u64)t_int);
	*t_hold = ret == 1 ? tmp_hold_t : *t_hold;
	return ret;
}

long gnsst2ts(char *ptr) {
	char timestr[5];
	struct tm t = {};
	t.tm_isdst = -1;
	VAL2INT(timestr,t.tm_year,&ptr[0],4);
	t.tm_year -= 1900;
	VAL2INT(timestr,t.tm_mon,&ptr[4],2);
	t.tm_mon--;
	VAL2INT(timestr,t.tm_mday,&ptr[6],2);
	VAL2INT(timestr,t.tm_hour,&ptr[8],2);
	VAL2INT(timestr,t.tm_min,&ptr[10],2);
	VAL2INT(timestr,t.tm_sec,&ptr[12],2);
//	char buffer[26];
//	strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", &t);
//	printf("%s\n",buffer);
	return mktime(&t);
}

int ts2time( long timestamp, char *buf) {
	time_t rawtime = timestamp;
    struct tm ts;
    ts = *localtime(&rawtime);
    strftime(buf, 80, "%H:%M:%S %Z", &ts);
    return 0;
}

int ts2date( long timestamp, char *buf) {
	time_t rawtime = timestamp;
    struct tm ts;
    ts = *localtime(&rawtime);
    strftime(buf, 100, "%A %d %B %Y", &ts);
    return 0;
}

int ts2datetime( long timestamp, char *buf) {
	time_t rawtime = timestamp;
    struct tm ts;
    ts = *localtime(&rawtime);
    strftime(buf, 100, "%A %d %B %Y %H:%M:%S %Z", &ts);
    return 0;
}

void save_reboot_reason( uint8_t reason ) {
	// Save reboot reason to flash
	nvs_handle my_ncs_handle;
	int err = nvs_open("storage", NVS_READWRITE, &my_ncs_handle);
	if ( err != ESP_OK) {
		ESP_LOGE(TAG ,"Error (%s) opening NVS handle!\n", esp_err_to_name(err) );
	}
	else {
		// Write
		ESP_LOGD(TAG, "Save reboot reason");
		if ( (err = nvs_set_u8(my_ncs_handle, "btr", reason )) != ESP_OK ) {
			ESP_LOGE(TAG, "fail writing reboot reason");
		}
		ESP_LOGD(TAG, "Committing updates in NVS");
		if ( (err = nvs_commit(my_ncs_handle)) != ESP_OK ) {
			ESP_LOGE(TAG, "fail committing NVS count updates ");
		}
		// Close
		nvs_close(my_ncs_handle);
	}
}

int id_from_mac(void) {
	uint8_t macaddr[6];
	esp_efuse_mac_get_default(macaddr);
	int acc = 0;
	for (int i = 0 ; i < 6 ; i++) {
		acc += macaddr[i];
	}
	return acc;
}

int set_gpio_output( u32 pin, u8 pup, u8 pdw, u8 intr ) {
	// Initialize output GPIOs
	gpio_config_t io_conf;
	io_conf.intr_type = (gpio_int_type_t) intr;  			// disable interrupt
	io_conf.mode = GPIO_MODE_OUTPUT;   						// set as output mode
	io_conf.pin_bit_mask = (1ULL << pin); 					// bit mask of the pins that you want to set
	io_conf.pull_down_en = (gpio_pulldown_t) pdw; 			// disable pull-down mode
	io_conf.pull_up_en = (gpio_pullup_t) pup;   			// disable pull-up mode
	return gpio_config(&io_conf);    						//configure GPIO with the given settings
}

int set_gpio_input( u32 pin, u8 pup, u8 pdw, u8 intr ) {
	// Initialize output GPIOs
	gpio_config_t io_conf;
	io_conf.intr_type = (gpio_int_type_t) intr;  			// disable interrupt
	io_conf.mode = GPIO_MODE_INPUT;   						// set as output mode
	io_conf.pin_bit_mask = (1ULL << pin); 					// bit mask of the pins that you want to set
	io_conf.pull_up_en = (gpio_pullup_t) pup;   			// disable pull-up mode
	io_conf.pull_down_en = (gpio_pulldown_t) pdw; 			// disable pull-down mode
	return gpio_config(&io_conf);    						//configure GPIO with the given settings
}

void get_mac_str(char macstr[]) {
	uint8_t macaddr[6];
	esp_efuse_mac_get_default(macaddr);
	macstr[sprintf(macstr, MYMACSTR, MAC2STR(macaddr))] = 0;
	ESP_LOGI(TAG, "device mac address: " MACSTR " [%s]", MAC2STR(macaddr), macstr );
}

float rand_int_decimal( u32 interval, u32 decimal) {
	float div = 1;
	while( decimal > 0 ) {
		div *= 10;
		decimal--;
	}
	interval *= div;
	return (float)(rand()%interval)/(div);
}
