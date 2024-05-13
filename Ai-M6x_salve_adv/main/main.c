/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-01-23
 *
 * @copyright Copyright (c) 2024
 *
*/
#include <FreeRTOS.h>
#include "task.h"
#include "board.h"
#include "bluetooth.h"
#include "btble_lib_api.h"
#include "bl616_glb.h"
#include "rfparam_adapter.h"
#include "bflb_mtd.h"
#include "easyflash.h"
#include "log.h"
#include "ble_interface.h"
#define DBG_TAG "MAIN"

int main(void)
{

    board_init();
    // aiio_log_init();

    if (0 != rfparam_init(0, NULL, 0)) {
        LOG_E("PHY RF init failed!\r\n");
        return 0;
    }
    ai_ble_start();
    vTaskStartScheduler();
    while (1) {

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
