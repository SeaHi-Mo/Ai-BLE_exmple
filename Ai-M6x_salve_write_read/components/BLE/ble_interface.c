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

#define SERVER_UUID_16BIT  BT_UUID_DECLARE_16(0X9011) //服务器UUID
#define NOTIFY_UUID_16BIT  BT_UUID_DECLARE_16(0X9012) //特征1 UUID
#define WRITER_UUID_16BIT  BT_UUID_DECLARE_16(0X9013) //特征2  UUID

// static void ble_ccc_cfg_changed(const struct bt_gatt_attr* attr, u16_t value);
static ssize_t ble_uuid_write_val(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, u16_t len, u16_t offset, u8_t flags);
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

/* 定义一个服务，这个服务有两个特性  */
static struct bt_gatt_attr salve_uuid_server[] = {
    /* 服务 UUID */
    BT_GATT_PRIMARY_SERVICE(SERVER_UUID_16BIT),
    /* 可读可通知的特征 */
    BT_GATT_CHARACTERISTIC(NOTIFY_UUID_16BIT,BT_GATT_CHRC_NOTIFY,BT_GATT_PERM_READ,NULL,NULL,NULL),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    /* 可读可写属性的特征 并声明接收回调函数*/
    BT_GATT_CHARACTERISTIC(WRITER_UUID_16BIT,BT_GATT_CHRC_WRITE,BT_GATT_PERM_WRITE,NULL,ble_uuid_write_val,NULL),
};

/**
 * @brief
 *
 * @param conn
 * @param attr
 * @param buf
 * @param len
 * @param offset
 * @param flags
 * @return ssize_t
*/
static ssize_t ble_uuid_write_val(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, u16_t len, u16_t offset, u8_t flags)
{
    uint8_t* recv_buffer;
    recv_buffer = pvPortMalloc(sizeof(uint8_t) * len);
    memcpy(recv_buffer, buf, len);
    LOG_D("recv ble data len= %d:%s", len, recv_buffer);
    for (size_t i = 0; i < len; i++)
    {
        LOG_RI("0x%x ", recv_buffer[i]);
    }
    printf("\r\n");
    vPortFree(recv_buffer);
    /* 给手机发送数据 */
    bt_gatt_notify(conn, &salve_uuid_server[2], "Hello Master", strlen("Hello Master"));
    return len;
}
/**
 * @brief
 *
 * @param attr
 * @param value
*/
static void ble_ccc_cfg_changed(const struct bt_gatt_attr* attr, u16_t value)
{
    char* str = "disabled";

    if (value == BT_GATT_CCC_NOTIFY)
    {
        str = "notify";
    }
    else if (value == BT_GATT_CCC_INDICATE)
    {
        str = "indicate";
    }

    LOG_I("[BLE] ccc change %s", str);
}

/* 创建 BLE 服务接口*/
static struct bt_gatt_service ble_uuid_server = BT_GATT_SERVICE(salve_uuid_server);

static int ble_salve_adv();
static void _connected(struct bt_conn* conn, u8_t err);
static void _disconnected(struct bt_conn* conn, u8_t reason);

static struct bt_conn_cb conn_callbacks = {
    .connected = _connected,
    .disconnected = _disconnected,
};

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
        LOG_I("BD_ADDR:(MSB)%02x:%02x:%02x:%02x:%02x:%02x(LSB)",
               bt_addr.a.val[5], bt_addr.a.val[4], bt_addr.a.val[3], bt_addr.a.val[2], bt_addr.a.val[1], bt_addr.a.val[0]);
        //蓝牙启动完成之后发送广播包
        ble_salve_adv();
    }
}
/**
 * @brief 连接成功回调
 *
 * @param conn
 * @param err
*/
static void _connected(struct bt_conn* conn, u8_t err)
{
    if (err) {
        LOG_E("connected err:%d", err);
        bt_conn_unref(conn);
        return;
    }
    LOG_I("connected：现在是连接态");
    bt_le_adv_stop();
    return;
}
/**
 * @brief
 *
 * @param conn
 * @param reason
*/
static void _disconnected(struct bt_conn* conn, u8_t reason)
{
    LOG_W("disconnected, 此时是就绪态reason:%d", reason);
    LOG_I("切换到 广播态");
    int err = bt_le_adv_start(BT_LE_ADV_CONN, salve_adv, ARRAY_SIZE(salve_adv), salve_rsp, ARRAY_SIZE(salve_rsp));
    if (err)
        LOG_E("[BLE] adv fail(err %d)", err);
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
    bt_conn_cb_register(&conn_callbacks);
    conn_callbacks._next = NULL;
    /* 注册BLE 服务*/
    int ret = bt_gatt_service_register(&ble_uuid_server);

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