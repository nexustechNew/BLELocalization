#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "bsp.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define BSP_INDICATE_ADVERTISING  4 
#define LED_PIN 18

#define APP_BLE_CONN_CFG_TAG            1                                  
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(1000, UNIT_0_625_MS) 
#define APP_BEACON_INFO_LENGTH          0x17                               
#define APP_ADV_DATA_LENGTH             0x15                               
#define APP_DEVICE_TYPE                 0x02                               
#define APP_MEASURED_RSSI               0xC3                               
#define APP_COMPANY_IDENTIFIER          0x0059                             
#define APP_MAJOR_VALUE                 0x01, 0x02                         
#define APP_MINOR_VALUE                 0x03, 0x04                         
#define APP_BEACON_UUID                 0x8A, 0x0A, 0x42, 0xE1, \
                                        0x53, 0x0E, 0x47, 0x54, \
                                        0xA9, 0xBA, 0x26, 0xA0, \
                                        0x88, 0x6F, 0x2F, 0x30            
#define DEAD_BEEF                       0xDEADBEEF                         

static ble_gap_adv_params_t m_adv_params;                                  
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; 
static uint8_t              m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  
static bool                 m_advertising = false;                         
static app_timer_id_t m_led_timer_id;  

static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = NULL,
        .len    = 0
    }
};

static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =                    
{
    APP_DEVICE_TYPE,     
    APP_ADV_DATA_LENGTH, 
    APP_BEACON_UUID,     
    APP_MAJOR_VALUE,     
    APP_MINOR_VALUE,     
    APP_MEASURED_RSSI    
};

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void stop_led_timer(void)
{
    ret_code_t err_code = app_timer_stop(m_led_timer_id);
    APP_ERROR_CHECK(err_code);
}

static void tx_power_set(int8_t tx_power)
{
    ret_code_t err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, tx_power);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("TX Power set to %d dBm", tx_power);
}

static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
    manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;

    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    memset(&m_adv_params, 0, sizeof(m_adv_params));
    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;       

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

static void advertising_start(void)
{
    if (!m_advertising)
    {
        ret_code_t err_code;
        err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
        APP_ERROR_CHECK(err_code);
        tx_power_set(-16);
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        m_advertising = true;
    }
}

static void advertising_stop(void)
{
    if (m_advertising)
    {
        ret_code_t err_code;
        err_code = sd_ble_gap_adv_stop(m_adv_handle);
        APP_ERROR_CHECK(err_code);
        m_advertising = false;
    }
}

void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            advertising_start();
            NRF_LOG_INFO("Reklam baslatildi.");
            printf("reklam basladi");
            break;

        case BSP_EVENT_KEY_1:
            advertising_stop();
            NRF_LOG_INFO("Reklam durduruldu.");
            printf("reklam durdu");
            break;
        default:
            break;
    }
}

static void ble_stack_init(void)
{
    ret_code_t err_code;
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}

static void leds_init(void)
{
    ret_code_t err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);
}

static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

int main(void)
{
    timers_init();
    leds_init();
    power_management_init();
    ble_stack_init();
    advertising_init();
    NRF_LOG_INFO("Beacon started.");
    for (;; )
    {
        idle_state_handle();
    }
}
