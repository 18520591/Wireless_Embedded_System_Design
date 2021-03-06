#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

static esp_ble_adv_params_t ble_adv_params = {
	
	.adv_int_min = 0x20,
	.adv_int_max = 0x40,
	.adv_type = ADV_TYPE_SCAN_IND,
	.own_addr_type  = BLE_ADDR_TYPE_PUBLIC,
	.channel_map = ADV_CHNL_ALL,
	.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t adv_name[14] = {0x0D,0x09,0x4c,0x6f,0x6e,0x67,0x2d,0x54,0x68,0x65,0x2d,0x44,0x61,0x74};
static uint8_t adv_data[28] = {0x1B,0x16,0x31,0x38,0x35,0x32,0x31,0x34,0x32,0x33,0x2d,0x31,0x38,0x35,0x32,0x31,0x30,0x32,0x31,0x2d,0x31,0x38,0x35,0x32,0x30,0x35,0x39,0x31};

bool adv_data_set = false;
bool scan_rsp_data_set = false;
								   
// GAP callback
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
			
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: 
				
			printf("ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT\n");
			adv_data_set = true;
			if(scan_rsp_data_set) esp_ble_gap_start_advertising(&ble_adv_params);
			break;
			
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: 
				
			printf("ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT\n");
			adv_data_set = true;
			if(scan_rsp_data_set) esp_ble_gap_start_advertising(&ble_adv_params);
			break;

		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
		
			printf("ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT\n");
			scan_rsp_data_set = true;
			if(adv_data_set) esp_ble_gap_start_advertising(&ble_adv_params);
			break;
			
		case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
		
			printf("ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT\n");
			scan_rsp_data_set = true;
			if(adv_data_set) esp_ble_gap_start_advertising(&ble_adv_params);
			break;
		
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			
			printf("ESP_GAP_BLE_ADV_START_COMPLETE_EVT\n");
			if(param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
				printf("Advertising started\n\n");
			}
			else printf("Unable to start advertising process, error code %d\n\n", param->scan_start_cmpl.status);
			break;
	
		default:
		
			printf("Event %d unhandled\n\n", event);
			break;
	}
}


void app_main() {
	
	printf("BT broadcast\n\n");
	
	// set components to log only errors
	esp_log_level_set("*", ESP_LOG_ERROR);
	
	// initialize nvs
	ESP_ERROR_CHECK(nvs_flash_init());
	printf("- NVS init ok\n");
	
	// release memory reserved for classic BT (not used)
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
	printf("- Memory for classic BT released\n");
	
	// initialize the BT controller with the default config
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
	printf("- BT controller init ok\n");
	
	// enable the BT controller in BLE mode
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
	printf("- BT controller enabled in BLE mode\n");
	
	// initialize Bluedroid library
	esp_bluedroid_init();
    esp_bluedroid_enable();
	printf("- Bluedroid initialized and enabled\n");
	
	// register GAP callback function
	ESP_ERROR_CHECK(esp_ble_gap_register_callback(esp_gap_cb));
	printf("- GAP callback registered\n");
	
	// configure the adv data
	ESP_ERROR_CHECK(esp_ble_gap_set_device_name("ESP32_ScanRsp"));
	ESP_ERROR_CHECK(esp_ble_gap_config_adv_data_raw(adv_name, 14));
	printf("- ADV name configured\n");
	
	// configure the adv response data
	ESP_ERROR_CHECK(esp_ble_gap_config_scan_rsp_data_raw(adv_data, 28));
	printf("- adv data configured\n\n");
}
