/**
 * @file ble_interface.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
*/
#include <stdio.h>
#include <string.h>
#include "bluetooth.h"
#include "hci_driver.h"
#include "hci_core.h"
#include "conn.h"
#include "conn_internal.h"
#include "gatt.h"
#include "ble_interface.h"

#ifdef CONFIG_Ai_M6x
#define DBG_TAG "BLE"

#include "btble_lib_api.h"
#include "log.h"
#endif
#ifdef CONFIG_Ai_WB2
#include "ble_lib_api.h"
#endif

//定义蓝牙名称
#define ble_slave_name "Ai-M61-BLE"
/*  定义广播数据 总子节数不得超过31 byte  */
static const struct bt_data salve_adv[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS,(BT_LE_AD_GENERAL| BT_LE_AD_NO_BREDR)),  //数据头占用 2 byte
    // BT_DATA(BT_DATA_NAME_COMPLETE,ble_slave_name,sizeof(ble_slave_name)-1),//第二个数据，定义名称
};
/*  定义扫描响应数据 总字节不得超过31byte*/
static const struct bt_data salve_rsp[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE,ble_slave_name,sizeof(ble_slave_name)-1),//把设备名称放在扫描响应当中
};

/* 定义扫描响应数据 总字节数不得超过31 byte*/

static int ble_salve_adv();
/**
 * @brief 蓝牙启动回调
 *
 * @param err
*/
static void bt_enable_cb(int err)
{
    if (!err)
    {
        bt_addr_le_t bt_addr;
        bt_get_local_public_address(&bt_addr);
        // bt_addr.a.val[5] = 0x88;
        // bt_addr.a.val[4] = 0x88;
        // bt_addr.a.val[3] = 0x88;
        // bt_addr.a.val[2] = 0x88;
        // bt_addr.a.val[1] = 0x88;
        // bt_addr.a.val[0] = 0x88;
        LOG_I("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB)",
               bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);
        //蓝牙启动完成之后发送广播包
        ble_salve_adv();
    }
}

/**
 * @brief 启动BLE 协议栈
 *
*/
static void ble_stack_start(void)
{
    // Initialize BLE controller
#ifdef CONFIG_Ai_M6x  
    btble_controller_init(configMAX_PRIORITIES - 1);
#endif
#ifdef CONFIG_Ai_WB2
    ble_controller_init(configMAX_PRIORITIES - 1);
#endif
    // Initialize BLE Host stack
    hci_driver_init();
    bt_enable(bt_enable_cb);
}

static int ble_salve_adv()
{
    int err = -1;
    /*启动广播，赋值广播数据和扫描响应数据*/
    err = bt_le_adv_start(BT_LE_ADV_CONN, salve_adv, ARRAY_SIZE(salve_adv), salve_rsp, ARRAY_SIZE(salve_rsp));

    if (err)
    {
        LOG_E("[BLE] adv fail(err %d)", err);
        return -1;
    }
    return 0;
}

void ai_ble_start(void)
{
    ble_stack_start();
}