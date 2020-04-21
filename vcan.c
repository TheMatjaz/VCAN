/**
 * @file
 *
 * @copyright Copyright © 2020, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 * @license BSD 3-clause license.
 */

#include "vcan.h"

vcan_err_t vcan_init(vcan_bus_t* const bus)
{
    vcan_err_t err;
    if (bus == NULL)
    {
        err = VCAN_NULL_BUS;
    }
    else
    {
        memset(bus, 0, sizeof(vcan_bus_t));
        err = VCAN_OK;
    }
    return err;
}

vcan_err_t vcan_tx(vcan_bus_t* const bus,
                   const vcan_msg_t* const msg,
                   const vcan_node_t* const src_node)
{
    vcan_err_t err;
    if (bus == NULL)
    {
        err = VCAN_NULL_BUS;
    }
    else if (msg == NULL)
    {
        err = VCAN_NULL_MSG;
    }
    else
    {
        for (size_t i = 0; i < bus->connected; i++)
        {
            if (src_node != bus->nodes[i])
            {
                memcpy(&bus->nodes[i]->received_msg, msg, sizeof(vcan_msg_t));
                bus->nodes[i]->callback_on_rx(bus->nodes[i]);
            }
        }
        err = VCAN_OK;
    }
    return err;
}

vcan_err_t vcan_connect(vcan_bus_t* const bus,
                        vcan_node_t* const node)
{
    vcan_err_t err;
    if (bus == NULL)
    {
        err = VCAN_NULL_BUS;
    }
    else if (node == NULL)
    {
        err = VCAN_NULL_NODE;
    }
    else if (node->callback_on_rx == NULL)
    {
        err = VCAN_NULL_CALLBACK;
    }
    else if (bus->connected >= VCAN_MAX_CONNECTED_NODES)
    {
        err = VCAN_TOO_MANY_CONNECTED;
    }
    else
    {
        err = VCAN_OK;
        for (size_t i = 0; i < bus->connected; i++)
        {
            if (bus->nodes[i] == node)
            {
                err = VCAN_ALREADY_CONNECTED;
            }
        }
        if (err == VCAN_OK)
        {
            bus->nodes[bus->connected++] = node;
        }
    }
    return err;
}

vcan_err_t vcan_disconnect(vcan_bus_t* const bus,
                           const vcan_node_t* const node)
{
    vcan_err_t err;
    if (bus == NULL)
    {
        err = VCAN_NULL_BUS;
    }
    else if (node == NULL)
    {
        err = VCAN_NULL_NODE;
    }
    else
    {
        err = VCAN_NODE_NOT_FOUND;
        for (size_t i = 0; i < bus->connected; i++)
        {
            if (bus->nodes[i] == node)
            {
                const size_t nodes_to_shift = bus->connected - i - 1;
                memmove(&bus->nodes[i],
                        &bus->nodes[i + 1],
                        nodes_to_shift * sizeof(vcan_node_t*));
                err = VCAN_OK;
                break;
            }
        }
    }
    return err;
}
