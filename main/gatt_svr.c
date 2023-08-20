#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "blehr_sens.h"
#include "esp_log.h"
#include "rgb_led.h"


#define TAG     "BLE_MUTLI_CONN_CENT_SVC"

static uint16_t gatt_svr_chr_val_handle;
static const char *manuf_name = "Apache Mynewt ESP32 devkitC";
static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);


static int
gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt,
                void *arg);


static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { 
            
            {
                /* Characteristic: * Manufacturer name */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID_READ),
                .access_cb = gatt_svr_chr_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                /*** This characteristic can be subscribed to by writing 0x00 and 0x01 to the CCCD ***/
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID_WRITE),
                .access_cb = gatt_svc_access,

                .flags = BLE_GATT_CHR_F_WRITE,

                .val_handle = &gatt_svr_chr_val_handle,
            }, {
                0, /* No more characteristics in this service. */
            }
        },
    },

    {
        0, /* No more services. */
    },
};

static int
gatt_svr_chr_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MANUFACTURER_NAME_UUID_READ) {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}


/**
 * Access callback whenever a characteristic/descriptor is read or written to.
 * Here reads and writes need to be handled.
 * ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor,
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
 * attr_handle give the value handle of the attribute being accessed.
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 **/

static int
gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint8_t data[10];
    uint8_t len;
    struct os_mbuf *om;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        ESP_LOGI(TAG, "Characteristic write; conn_handle=%d", conn_handle);
        if (attr_handle == gatt_svr_chr_val_handle) {
            om = ctxt->om;
            len = os_mbuf_len(om);
            len = len < sizeof(data) ? len : sizeof(data);
            assert(os_mbuf_copydata(om, 0, len, data) == 0);
            ESP_LOG_BUFFER_HEX(TAG, data, len);
            /* Send the received data to all of peers. */
            //peer_traverse_all(gatt_svc_send_to_peers, om);
            handle_led(data);
            return 0;
        }
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    return BLE_ATT_ERR_UNLIKELY;
}


int
gatt_svr_init(void)
{
    int rc;


    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}