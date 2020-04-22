/**
 * @file
 *
 * VCAN is a tiny Virtual CAN and CAN-FD bus.
 *
 * Especially useful for debugging and testing without using actual
 * CAN-connected devices, VCAN is a tiny C library that allows the user to
 * connect virtual nodes on a virtual bus and make them react whenever
 * someone transmits a message on the bus.
 *
 * After the transmission, each node obtains a copy of the message
 * and a callback on each node is called to warn the node of a message being
 * received.
 *
 * **Limitations**
 *
 * VCAN is simple, synchronous and not thread safe. It does not simulate
 * transmission errors, collisions, arbitration, etc. just pure data transfer.
 * Callbacks should be fast.
 *
 * ... but you are free to alter it to your specific needs!
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

/** VCAN version using semantic versioning. */
#define VCAN_VERSION "2.0.0"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/** Max payload size of a CAN message in bytes. */
#define VCAN_DATA_MAX_LEN 64
/** MAx amount of virtual nodes connected to the virtual bus. */
#define VCAN_MAX_CONNECTED_NODES 16

/** VCAN error codes. */
typedef enum
{
    /** Successfully completed. */
            VCAN_OK = 0,
    /** The bus argument is NULL. */
            VCAN_NULL_BUS = 1,
    /** The message argument is NULL. */
            VCAN_NULL_MSG = 2,
    /** The node argument is NULL. */
            VCAN_NULL_NODE = 3,
    /** The callback within the node is NULL. */
            VCAN_NULL_CALLBACK = 4,

    /**
     * Max amount of connected nodes reached.
     * Consider increasing #VCAN_MAX_CONNECTED_NODES.
     */
            VCAN_TOO_MANY_CONNECTED = 5,

    /** This node is not connected to the bus, so it cannot be disconnected. */
            VCAN_NODE_NOT_FOUND = 6,
    /** The node is already connected to the bus. */
            VCAN_ALREADY_CONNECTED = 7,
} vcan_err_t;

/** Message to transmit or receive. */
typedef struct
{
    /** The CAN ID - unused by VCAN. */
    uint32_t id;

    /** Used bytes in the \p data field. */
    uint32_t len;

    /** Payload. */
    uint8_t data[VCAN_DATA_MAX_LEN];
} vcan_msg_t;

/**
 * Virtual node.
 *
 * Contains a copy of the last received message and a callback function, which
 * is called whenever a message is received.
 */
struct vcan_node
{
    /**
     * Callback called when a message has been written into \p received_msg.
     *
     * NOTE: the callback should be simple and fast, e.g. copying the message
     * somewhere else and triggering a flag.
     *
     * @param node the address of this node. The whole node itself is passed
     * to the callback, so the function has access to itself (for recursion),
     * to any custom data it may need and of course to the just received
     * message.
     */
    void (* callback_on_rx)(struct vcan_node* node, const vcan_msg_t* msg);

    /** Any data the callback may need, such as a flag to trigger on
     * reception. Can be NULL. */
    void* other_custom_data;

    /** Identifier of the node. Can be set to anything, VCAN does not use it. */
    uint32_t id;
};

/**
 * Virtual node.
 *
 * Contains a copy of the last received message and a callback function, which
 * is called whenever a message is received.
 */
typedef struct vcan_node vcan_node_t;

/**
 * Virtual bus.
 *
 * Contains a list of nodes connected to it.
 */
typedef struct
{
    /** The message just transmitted over the bus. */
    vcan_msg_t received_msg;

    /** Nodes to deliver new messages to. */
    vcan_node_t* nodes[VCAN_MAX_CONNECTED_NODES];

    /** Amount of nodes. */
    size_t connected;
} vcan_bus_t;

/**
 * Initialises the bus.
 *
 * @param bus not NULL
 * @return
 * - #VCAN_NULL_BUS on \p bus being NULL
 * - #VCAN_OK otherwise
 */
vcan_err_t vcan_init(vcan_bus_t* bus);

/**
 * Attaches a new node to the bus, enabling it to receive any transmitted
 * message.
 *
 * When someone transmits a message, the node will get a copy into
 * \p node->received_msg and its callback will be called, passing
 * the node itself as its only argument.
 *
 * @param bus not NULL
 * @param node not NULL, with callback not NULL
 * @return
 * - #VCAN_NULL_BUS on \p bus being NULL
 * - #VCAN_NULL_NODE on \p node being NULL
 * - #VCAN_NULL_CALLBACK on \p node->callback_on_rx being NULL
 * - #VCAN_TOO_MANY_CONNECTED when there number of already connected nodes
 *   to the bus the maximum. Increase #VCAN_MAX_CONNECTED_NODES if required.
 * - #VCAN_ALREADY_CONNECTED when the node is already connected to the bus,
 *   there is nothing to be done.
 * - #VCAN_OK otherwise
 */
vcan_err_t vcan_connect(vcan_bus_t* bus, vcan_node_t* node);

/**
 * Detaches a node from the bus, disabling it from receiving any further
 * messages.
 *
 * @param bus not NULL
 * @param node not NULL
 * @return
 * - #VCAN_NULL_BUS on \p bus being NULL
 * - #VCAN_NULL_NODE on \p node being NULL
 * - #VCAN_NODE_NOT_FOUND the node was not connected to this bus, so there
 *   is nothing to do
 * - #VCAN_OK otherwise
 */
vcan_err_t vcan_disconnect(vcan_bus_t* bus, const vcan_node_t* node);

/**
 * Sends a copy of the message to every connected node and calls every nodes's
 * callback to notify them.
 *
 * If you include a transmitting node, that one is excluded from the reception.
 *
 * The callbacks must be fast in order to make this function perform quick
 * enough. Before transmitting the next message, the user should take care
 * that each virtual node has finished processing the message (e.g. copying to
 * another location), so the next transmit does not overwrite the
 * unprocessed message in the nodes.
 *
 * @param bus not NULL
 * @param msg not NULL
 * @param src_node can be NULL
 * @return
 * - #VCAN_NULL_BUS on \p bus being NULL
 * - #VCAN_NULL_MSG on \p msg being NULL
 * - #VCAN_OK otherwise
 */
vcan_err_t vcan_tx(vcan_bus_t* bus,
                   const vcan_msg_t* msg,
                   const vcan_node_t* src_node);

#ifdef __cplusplus
}
#endif

#endif  /* VCAN_H */
