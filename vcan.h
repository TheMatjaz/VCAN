/**
 * @file
 *
 * @copyright Copyright © 2020, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 * @license BSD 3-clause license.
 */

#ifndef VCAN_H
#define VCAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define VCAN_DATA_MAX_LEN 64
#define VCAN_MAX_NODES 32

typedef uint32_t vcan_id_t;
typedef uint32_t vcan_len_t;

typedef enum
{
    VCAN_OK = 0,
    VCAN_NULL_BUS,
    VCAN_NULL_MSG,
    VCAN_NULL_NODE,
    VCAN_NULL_CALLBACK,
    VCAN_TOO_MANY_CONNECTED,
    VCAN_NOT_FOUND,
} vcan_err_t;

typedef struct
{
    vcan_id_t id;
    vcan_len_t len;
    uint8_t data[VCAN_DATA_MAX_LEN];
} vcan_msg_t;

struct vcan_node
{
    void (*callback_on_rx)(struct vcan_node* node);
    void* other_custom_data;
    vcan_msg_t received_msg;
};
typedef struct vcan_node vcan_node_t;

typedef struct
{
    vcan_node_t* nodes[VCAN_MAX_NODES];
    size_t connected;
} vcan_bus_t;

// Inits bus. crashes on NULL arg
vcan_err_t vcan_init(vcan_bus_t* bus);
// Publishes on the bus for every connected to receive. crashes on NULL args
vcan_err_t vcan_tx(vcan_bus_t* bus, const vcan_msg_t* msg);
// Connects new node to the bus. On reception event, the data is copied
// into msg and the callback is called.
vcan_err_t vcan_connect(vcan_bus_t* bus, vcan_node_t* node);
// Disconnects a node, thus unsubscribing the callback.
vcan_err_t vcan_disconnect(vcan_bus_t* bus, const vcan_node_t* node);

#ifdef __cplusplus
}
#endif

#endif  /* VCAN_H */
