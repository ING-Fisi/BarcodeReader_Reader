/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "driver/gpio.h"
//#include <cstring>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "fsntp.h"
#include "WifiConnect.h"
#include "utility.h"

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "protocol_examples_common.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "esp_http_client.h"

void http_rest_with_url(void);

#define GOOGLE_SNTP "time.google.com"
#define DEFAULT_SNTP_SERVER GOOGLE_SNTP // Select one of the previous servers


#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "HTTP_CLIENT";

#define CONFIG_EXAMPLE_HTTP_ENDPOINT "192.168.1.100:8080"


static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

uint8_t* data;
size_t length=0;


//************************* RELE ******************************//
#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)



//char* stringData;

//void uint8_to_string(uint8_t* data, size_t length, char* stringData) {
//	for(size_t i=0; i<length; i++){
//		stringData[i]=(char)data[i];
//	}
//	stringData[length]='\0';
//}

void init(void) {
	const uart_config_t uart_config = {
			.baud_rate = 9600,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_APB,
	};
	// We won't use a buffer for sending data.
	uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_param_config(UART_NUM_2, &uart_config);
	uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

int sendData(const char* logName, const char* data)
{
	const int len = strlen(data);
	const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
	ESP_LOGI(logName, "Wrote %d bytes", txBytes);
	return txBytes;
}

static void tx_task(void *arg)
{
	static const char *TX_TASK_TAG = "TX_TASK";
	esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
	while (1) {
		sendData(TX_TASK_TAG, "Hello world");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

static void rx_task(void *arg)
{
	static const char *RX_TASK_TAG = "RX_TASK";
	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	data = (uint8_t*) malloc(RX_BUF_SIZE+1); //data = (uint8_t*) malloc(RX_BUF_SIZE+1); //uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
	while (1) {
		const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
		if (rxBytes > 0) {
			data[rxBytes] = 0;
			ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
			ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

			length=(size_t)rxBytes;

			http_rest_with_url();
		}
	}
	free(data);
}

char stringResp[100];

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{


	static char *output_buffer;  // Buffer to store response of http request from event handler
	static int output_len;       // Stores number of bytes read
	switch(evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d [%s] [%s] [%s]", evt->data_len, (char*)evt->data,(char*)evt->header_key,(char*)evt->header_value);



		int data_len = evt->data_len;
		memset(stringResp,0,sizeof(stringResp));
		strncpy(stringResp, (char*)evt->data, data_len);
		stringResp[data_len]='\0';
		ESP_LOGI(TAG, "stringResp = [%s]", stringResp);

		if(strncmp(stringResp,"0xQRENABLED",data_len)==0){
			ESP_LOGI(TAG, "QR CODE ENABLED");
			gpio_set_level(GPIO_OUTPUT_IO_0, false);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			gpio_set_level(GPIO_OUTPUT_IO_0, true);
		}

		if(strncmp(stringResp,"0xQRDISABLED",data_len)==0){
			ESP_LOGI(TAG, "QR CODE DISABLED");
		}




		// Clean the buffer in case of a new request
		//            if (output_len == 0 && evt->user_data) {
		//                // we are just starting to copy the output data into the use
		//                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
		//            }
		//            /*
		//             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
		//             *  However, event handler can also be used in case chunked encoding is used.
		//             */
		//            if (!esp_http_client_is_chunked_response(evt->client)) {
		//                // If user_data buffer is configured, copy the response into the buffer
		//                int copy_len = 0;
		//                if (evt->user_data) {
		//                    // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
		//                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
		//                    if (copy_len) {
		//                        memcpy(evt->user_data + output_len, evt->data, copy_len);
		//                    }
		//                } else {
		//                    int content_len = esp_http_client_get_content_length(evt->client);
		//                    if (output_buffer == NULL) {
		//                        // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
		//                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
		//                        output_len = 0;
		//                        if (output_buffer == NULL) {
		//                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
		//                            return ESP_FAIL;
		//                        }
		//                    }
		//                    copy_len = MIN(evt->data_len, (content_len - output_len));
		//                    if (copy_len) {
		//                        memcpy(output_buffer + output_len, evt->data, copy_len);
		//                    }
		//                }
		//                output_len += copy_len;
		//            }

		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
		if (output_buffer != NULL) {
			// Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
			// ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
			free(output_buffer);
			output_buffer = NULL;
		}
		output_len = 0;
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		//            int mbedtls_err = 0;
		//            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
		//            if (err != 0) {
		//                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
		//                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
		//            }
		//            if (output_buffer != NULL) {
		//                free(output_buffer);
		//                output_buffer = NULL;
		//            }
		//            output_len = 0;
		break;
		//        case HTTP_EVENT_REDIRECT:
		//            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
		//            esp_http_client_set_header(evt->client, "From", "user@example.com");
		//            esp_http_client_set_header(evt->client, "Accept", "text/html");
		//            esp_http_client_set_redirection(evt->client);
		//            break;
	}
	return ESP_OK;
}

char macstr[20];


void http_rest_with_url(void)
{
	// Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
	// it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
	char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
	/**
	 * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
	 * If host and path parameters are not set, query parameter will be ignored. In such cases,
	 * query parameter should be specified in URL.
	 *
	 * If URL as well as host and path parameters are specified, values of host and path will be considered.
	 */
	esp_http_client_config_t config = {
			.host = CONFIG_EXAMPLE_HTTP_ENDPOINT,
			.path = "/get",
			.query = "esp",
			.event_handler = _http_event_handler,
			.user_data = local_response_buffer,        // Pass address of local buffer to get response
			.disable_auto_redirect = true,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	//size_t length =(size_t)rxBytes;
	char stringData[length+1];
	for(size_t i=0; i<length; i++){
		stringData[i]=(char)data[i];
	}
	stringData[length]='\0';
	ESP_LOGI(TAG,  "Letti %d bytes; stringData: '%s'", length, stringData);

	get_mac_str(macstr);

	// POST
	const char *post_data = "{\"field1\":\"value1\"}";

	char* str1="http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/";
	char* str2=stringData;
	char* macadd = macstr;
	size_t len = strlen(str1) + strlen(str2) + 1 + strlen(macadd) + 1;
	const char* result = (char*)malloc(len);
	strcpy(result,str1);
	strcat(result,str2);
	strcat(result,"_");
	strcat(result,macadd);
	esp_http_client_set_url(client, result);
	esp_http_client_set_method(client, HTTP_METHOD_POST);
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_post_field(client, post_data, strlen(post_data));
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK) {
		ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
				esp_http_client_get_status_code(client),
				esp_http_client_get_content_length(client));
	} else {
		ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
	}


	esp_http_client_cleanup(client);
}


void app_main(void)
{


	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());





	//**********************************************************//

	//zero-initialize the config structure.
	gpio_config_t io_conf = {};
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);


	gpio_set_level(GPIO_OUTPUT_IO_0, true);




	//	esp_http_client_config_t config = {
	//		.host = CONFIG_EXAMPLE_HTTP_ENDPOINT,
	//		.path = "/get",
	//		.query = "esp",
	//		.event_handler = _http_event_handler,
	//		.user_data = local_response_buffer,        // Pass address of local buffer to get response
	//		.disable_auto_redirect = true,
	//	};
	//	esp_http_client_handle_t client = esp_http_client_init(&config);

	ESP_ERROR_CHECK(fisi_example_connect());

	//***************************************************************************//
	while (1) {
		if ( sntp_init_time( DEFAULT_SNTP_SERVER, 20) != 0 ) { // UNIFI_SNTP
			//ESP_LOGW(TAG,"fail obtaining time from specified SNTP server.");
		}
		else {
			break;
		}
	}

	//*************************************************************************//

//	bool toggle = false;
//	while(1) {
//		vTaskDelay(1000 / portTICK_RATE_MS);
//
//		if(toggle == false)
//		{
//			toggle = true;
//		}
//		else
//		{
//			toggle = false;
//		}
//
//
//
//	}






	init();
	xTaskCreate(rx_task, "uart_rx_task", 4096*2, NULL, configMAX_PRIORITIES, NULL);
	//xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
