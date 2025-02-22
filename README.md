# How to Design a Beacon?

## 1. Abstract
This document presents a **Bluetooth Low Energy (BLE) Beacon** project implemented using the **nRF52** series microcontroller. The project is designed to transmit advertising packets at predefined intervals, enabling uninterrupted data broadcasting. This beacon can be utilized in various applications, such as **indoor positioning, asset tracking, and proximity-based services**.

## 2. Requirements

### **Hardware:**
- **nRF52810 Development Kit** (or any other compatible nRF52 family device)

### **Software:**
- **Segger Embedded Studio (SES)** – Integrated Development Environment (IDE) for firmware development  
  - [Download](https://www.segger.com/products/development-tools/embedded-studio/)
- **nRF SDK (nRF5 SDK or nRF Connect SDK)** – Includes required libraries and drivers  
  - [Download](https://www.nordicsemi.com/Products/Development-software/nRF5-SDK)
- **nRF Connect for Desktop/Mobile** – Used for testing BLE communication  
  - Download: Available on **App Store & Play Store**

- **Suggestion:** 
  - First, install **Segger Embedded Studio**. Then, download the **nRF5 SDK** and extract the files from the archive. Next, navigate to the ***examples/ble_peripheral*** directory. Copy the ***ble_app_beacon*** folder and create an empty folder inside the examples directory. Paste the copied folder into the newly created one.

  - After that, open the copied **ble_app_beacon** folder and go to the ***pca100xx*** directory that corresponds to your nRF kit (mine is pca10040). Inside this folder, navigate   to the s1xx directory (mine is s132). Then, enter the ***ses*** folder. Finally, double-click the ***.emProject***(mine ble_app_beacon_pca10040_s132.emProject) file inside to open the project.
---

## 3. Libraries Used
The following libraries are included in the **nRF SDK** (*located in nrf_sdk/components/*):

### **i. Hardware Abstraction Layer (HAL)**
- `boards.c`, `bsp.c` → Provides board-specific support.
- `system_nrf52.c` → Contains system initialization and basic configurations.
- `nrf_drv_clock.c`, `nrfx_clock.c` → Clock management.
- `nrf_drv_uart.c`, `nrfx_uart.c`, `nrfx_uarte.c` → UART drivers.
- `nrfx_gpiote.c` → GPIO event management.
- `nrfx_prs.c` → Peripheral resource sharing.

### **ii. BLE (Bluetooth Low Energy) Libraries**
- `ble_advdata.c` → BLE advertising data configuration.
- `ble_srv_common.c` → Common BLE service operations.
- `nrf_sdh.c`, `nrf_sdh_ble.c`, `nrf_sdh_soc.c` → SoftDevice management.

### **iii. Timing and Scheduling Mechanisms**
- `app_timer2.c` → Software timers.
- `app_scheduler.c` → Task scheduling.
- `drv_rtc.c` → Real-time counter (RTC) management.

### **iv. Error Handling and Debugging**
- `app_error.c`, `app_error_handler_gcc.c`, `app_error_weak.c` → Error management.
- `hardfault_implementation.c` → Hardfault exception handler.
- `nrf_assert.c` → Assertion functions for debugging.
- `nrf_strerror.c` → Error message generation.

### **v. Memory and Data Structures**
- `nrf_atfifo.c`, `nrf_ringbuf.c` → FIFO and ring buffer management.
- `nrf_atomic.c`, `nrfx_atomic.c` → Atomic operations.
- `nrf_balloc.c` → Memory allocation management.
- `nrf_memobj.c` → Memory object management.
- `nrf_sortlist.c` → Sorted list management.

### **vi. Logging and Debug Output**
- `nrf_log_backend_rtt.c`, `nrf_log_backend_serial.c`, `nrf_log_backend_uart.c` → Log output backends.
- `nrf_log_default_backends.c`, `nrf_log_frontend.c`, `nrf_log_str_formatter.c` → Logging operations.
- `SEGGER_RTT.c`, `SEGGER_RTT_printf.c`, `SEGGER_RTT_Syscalls_SES.c` → Log output via SEGGER RTT.

### **vii. Platform-Specific and General Utility Libraries**
- `app_util_platform.c` → General utility functions.
- `nrf_fprintf.c`, `nrf_fprintf_format.c` → Print functions.
- `nrf_pwr_mgmt.c` → Power management.
- `nrf_section_iter.c` → Iteration over memory sections.

### **viii. Assembly and Other Utilities**
- `thumb_crt0.s` → C runtime startup code.
- `utf.c` → UTF character processing.

---

## 4. Project Structure
This project functions as a **BLE (Bluetooth Low Energy) beacon**, broadcasting advertisements with predefined **UUID, Major, and Minor** values. The system is optimized for low power consumption and includes **button-controlled operations** and **LED feedback mechanisms**.

### **i) BLE Advertising Management**
- Configuring **UUID, Major, and Minor** values to ensure proper advertisement visibility.
- Running BLE in **low-power mode** and implementing energy-efficient optimizations.
- Adjusting **TX power** to optimize signal range and control transmission power.

### **ii) Button-Controlled Advertising Management**
- Detecting **short press** and **long press** button events.
- Starting advertising on a short press and stopping it on another press.
- Using a **long press** to cancel advertisements after holding the button for a specific duration.

### **iii) LED Feedback**
- **Blinking the LED** when BLE advertising starts.
- **Turning off the LED** when advertising stops.

### **iv) Power Management and Optimization**
- Implementing **low-power modes** to minimize unnecessary power consumption.
- Dynamically adjusting **TX power** to optimize energy efficiency and extend battery life.

---

## 5. Function Descriptions
This section provides an overview of the key functions used in the project, along with their purpose and implementation details.

### **a) Core Functionalities**
#### `assert_nrf_callback`
- Handles assertion failures and forwards error details to `app_error_handler`.

#### `tx_power_set`
- Configures **BLE transmission power** (`tx_power`).
- Uses `sd_ble_gap_tx_power_set` for setting power levels.
- Includes error handling and debugging.

#### `advertising_init`
- Configures the BLE **advertising packet**.
- Prepares advertising data, sets parameters, and enables BLE stack configurations.

#### `advertising_start`
- Starts BLE advertising and applies necessary configurations.

#### `advertising_stop`
- Stops BLE advertising and resets related parameters.

#### `button_timer_handler`
- Detects **long-press events** to manage BLE advertising.

#### `bsp_event_handler`
- Handles button press events to **start/stop BLE advertising**.

#### `ble_stack_init`
- Initializes and configures the **BLE stack**.

#### `leds_init`
- Initializes **LEDs and buttons**.

#### `led_blink_handler`
- Controls **LED blinking behavior**.

#### `timers_init`
- Initializes timers for **button and LED control**.

#### `timers_start`
- Starts the **LED blink timer**.

#### `power_management_init`
- Initializes the **power management** module.

#### `idle_state_handle`
- Handles **idle states** to reduce power consumption.

---

## 6. Test & Debug

### **a) Debug Logs**
- Logging enabled via `NRF_LOG_INFO()`.
- Ensure the following in `sdk_config.h`:
  ```c
  #define NRF_LOG_ENABLED 1
  #define NRF_LOG_BACKEND_RTT_ENABLED 1
  ```
  
  ### **b) Test Scenarios**
  To verify core functionalities, execute the following test cases:
  
  #### **BLE Advertising Test**
  - **Press Button 1** → Does it **start advertising**?
  - **Press Button 2** → Does it **stop advertising**?
  - **Long-press Button 3** → Does it **stop advertising** after the required duration?
  - ![image](https://github.com/user-attachments/assets/616cd0de-cd48-4bf0-a50a-91661ec31c96)

  
  #### **LED Behavior**
  - When **advertising starts** → Does **LED 1 blink**?
  - When **advertising stops** → Does **LED 1 turn off** or **stay solid**?
  
  These test cases ensure that the BLE beacon operates correctly under different conditions, providing clear debugging insights for any potential issues.
  
  - ![image](https://github.com/user-attachments/assets/0dcc4405-9768-4b8f-809c-e5e39b1a589e)

  
  ---
  
  ### **c) TX Power Test (RSSI Measurement)**
  To verify the effectiveness of **TX power settings**, you can measure the **RSSI (Received Signal Strength Indicator)** using the **nRF Connect mobile app**.
  
  #### **Test Steps:**
  1. Open **nRF Connect** and start **scanning for BLE devices**.
  2. Locate your **BLE beacon** and observe its **RSSI value**.
  3. Modify the **tx_power** setting in your firmware or change the **physical distance** between the beacon and the mobile device.
  4. Restart the device and **scan for BLE signals again**.
  5. Compare the **new RSSI values** with previous measurements.
  
  #### **Expected Results:**
  - **Higher TX power** → **Stronger signal** (**higher RSSI value**).
  - **Lower TX power** → **Weaker signal** (**lower RSSI value**).
  
  This test ensures that **TX power adjustments** are functioning correctly and impacting **signal strength** as expected.
  
  
  
  

![image](https://github.com/user-attachments/assets/0c68a671-a0fe-4ec9-8be0-125c3def0657)               ![image](https://github.com/user-attachments/assets/d4d31872-fc7c-4402-aca3-e6e54837580e)










