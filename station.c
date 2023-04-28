// Connect to an AP and print out IP address through the wire

#include <esp/uart.h>
#include <stdio.h>
#include <espressif/esp_common.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
#include "ssid_config.h"

static void wifi_connect()
{	
	
	while (true)
	{
		struct ip_info ipconfig;
		wifi_get_ip_info(STATION_IF, &ipconfig);
		const TickType_t xDelay = 2500 / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}
}	

static void wifi(void *pvParameters)
{
	uint8_t status = 0;
	uint8_t retries = 30;
	struct sdk_station_config config = { WIFI_SSID, WIFI_PASS };

	printf("WiFi: connecting to WiFi\n\r");
	sdk_wifi_set_opmode(STATION_MODE);
	sdk_wifi_station_set_config(&config);

	while (1) {
		while ((status != STATION_GOT_IP) && (retries)) {
			status = sdk_wifi_station_get_connect_status();
			printf("%s: status = %d\n\r", __func__, status);
			if (status == STATION_WRONG_PASSWORD) {
				printf("WiFi: wrong password\n\r");
				break;
			} else if (status == STATION_NO_AP_FOUND) {
				printf("WiFi: AP not found\n\r");
				break;
			} else if (status == STATION_CONNECT_FAIL) {
				printf("WiFi: connection failed\r\n");
				break;
			}
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			--retries;
		}
		if (status == STATION_GOT_IP) {
			printf("WiFi: Connected\n\r");
			xSemaphoreGive(wifi_alive);
			taskYIELD();
		}

		while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
			xSemaphoreGive(wifi_alive);
			taskYIELD();
		}
		printf("WiFi: disconnected\n\r");
		sdk_wifi_station_disconnect();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void user_init()
{
	uart_set_baud(0, 115200);

	xTaskCreate(&wifi, "connect", 256, NULL, 2, NULL);
}
