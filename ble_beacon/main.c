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
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "ble_advertising.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define LONG_PUSH_TIMEOUT_MS  5000  
#define BSP_INDICATE_ADVERTISING  4 
#define APP_BLE_CONN_CFG_TAG            1                                  
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(1000, UNIT_0_625_MS) 
#define APP_BEACON_INFO_LENGTH          0x17                               
#define APP_ADV_DATA_LENGTH             0x15                               
#define APP_DEVICE_TYPE                 0x02                               
#define APP_MEASURED_RSSI               0xC3                               
#define APP_COMPANY_IDENTIFIER          0x0059                             
#define APP_MAJOR_VALUE                 0xC3, 0xB7                                                  
#define APP_MINOR_VALUE                 0xA2, 0x24                                                  
#define APP_BEACON_UUID                 0xC8, 0x70, 0xD9, 0x75, \
                                        0xAF, 0x4A, 0x4D, 0x66, \
                                        0xAB, 0xFF, 0xF4, 0xE5, \
                                        0xE8, 0x82, 0x39, 0xF7            
#define DEAD_BEEF                       0xDEADBEEF 


APP_TIMER_DEF(m_button_timer);
APP_TIMER_DEF(led_timer_id);


static ble_gap_adv_params_t m_adv_params;                                  
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; 
static uint8_t              m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  
static bool                 m_advertising = false;                         
static bool button3_pressed = false;
static uint32_t button3_press_time = 0;


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


static void tx_power_set(int8_t tx_power)
{
    ret_code_t err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, tx_power);
    APP_ERROR_CHECK(err_code);
    #ifdef DEBUG
      NRF_LOG_INFO("TX Power set to %d dBm", tx_power);
    #endif
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
        tx_power_set(0);
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        m_advertising = true;

        err_code = app_timer_start(led_timer_id, APP_TIMER_TICKS(1000), NULL);
        APP_ERROR_CHECK(err_code);
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

        err_code = app_timer_stop(led_timer_id);
        APP_ERROR_CHECK(err_code);
        nrf_gpio_pin_set(LED_1); 
    }
}


static void button_timer_handler(void *p_context)
{
    advertising_stop();
    #ifdef DEBUG
      NRF_LOG_INFO("Button3 - Pressed for 5 seconds, ad stopped.");
    #endif
}


static void bsp_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action == APP_BUTTON_PUSH)
    {
        if (pin_no == BUTTON_1) 
        {
            advertising_start();
            #ifdef DEBUG
              NRF_LOG_INFO("Button 1 - Advertising started.");
            #endif
        }
        else if (pin_no == BUTTON_2)
        {
            advertising_stop();
            #ifdef DEBUG
              NRF_LOG_INFO("Button 2 - Advertising stopped.");
            #endif
        }
        else if (pin_no == BUTTON_3) 
        {
            button3_pressed = true;
            button3_press_time = app_timer_cnt_get();
            #ifdef DEBUG
              NRF_LOG_INFO("Button 3 - Pressed, time recorded.");
            #endif
        }
    }
    else if (button_action == APP_BUTTON_RELEASE)
    {
        if (pin_no == BUTTON_3)
        {
            uint32_t elapsed_time = app_timer_cnt_diff_compute(app_timer_cnt_get(), button3_press_time);
            if (elapsed_time >= APP_TIMER_TICKS(LONG_PUSH_TIMEOUT_MS)) 
            {
                advertising_stop();
                #ifdef DEBUG
                  NRF_LOG_INFO("Button 3 - Pressed more than 5 seconds, adv cancelled.");
                #endif
            }
            else
            {
                #ifdef DEBUG
                  NRF_LOG_INFO("Button 3 - Pressed less than 5 seconds, adv continued.");
                #endif
            }
            button3_pressed = false;
        }
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


static app_button_cfg_t button_cfg[] =
{
    {BUTTON_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, bsp_event_handler},
    {BUTTON_2, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, bsp_event_handler},
    {BUTTON_3, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, bsp_event_handler}
};


static void leds_init(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);

    nrf_gpio_pin_clear(LED_1);
    nrf_gpio_pin_set(LED_2);


    err_code = app_button_init(button_cfg, BUTTONS_NUMBER, APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(err_code); 

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);   
}


static void led_blink_handler(void *p_context)
{
    nrf_gpio_pin_toggle(LED_1);
}


static void timers_init(void)
{
    app_timer_init();
    ret_code_t err_code = app_timer_create(&m_button_timer, APP_TIMER_MODE_SINGLE_SHOT, button_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&led_timer_id, APP_TIMER_MODE_REPEATED, led_blink_handler);
    APP_ERROR_CHECK(err_code);
}


static void timers_start(void)
{
    ret_code_t err_code;
    err_code = app_timer_start(led_timer_id, APP_TIMER_TICKS(1000), NULL);
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
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
    leds_init();
    timers_init();
    power_management_init();
    ble_stack_init();
    advertising_init();

    for (;; )
    {
        idle_state_handle();
    }
}
