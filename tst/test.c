/**
 * @file
 *
 * Unit test suite for VCAN.
 *
 * @copyright Copyright © 2020, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 * @license BSD 3-clause license.
 */

#include "atto.h"
#include "vcan.h"
#include <assert.h>
#include <inttypes.h>

static void test_init_null(void)
{
    vcan_err_t err = vcan_init(NULL);

    atto_eq(err, VCAN_NULL_BUS);
}

static void test_init_valid(void)
{
    vcan_bus_t bus;

    vcan_err_t err = vcan_init(&bus);

    atto_eq(err, VCAN_OK);
    atto_zeros((uint8_t*) &bus, sizeof(bus))
}

static void test_connect_null_bus(void)
{
    vcan_err_t err = vcan_connect(NULL, NULL);

    atto_eq(err, VCAN_NULL_BUS);
}

static void test_connect_null_node(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);

    err = vcan_connect(&bus, NULL);

    atto_eq(err, VCAN_NULL_NODE);
}

static void test_connect_null_callback(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = NULL,
            .other_custom_data = NULL,
    };

    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_NULL_CALLBACK);
}

static void does_nothing(vcan_node_t* node, const vcan_msg_t* msg)
{
    (void) node;
    (void) msg;
}

static void test_connect_valid(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
    };

    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_OK);
    atto_eq(bus.connected, 1);
    atto_eq(bus.nodes[0], &node);
}


static void test_connect_already_connected(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
    };

    err = vcan_connect(&bus, &node);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_ALREADY_CONNECTED);
    atto_eq(bus.connected, 1);
    atto_eq(bus.nodes[0], &node);
}

static void test_connect_max_reached(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
    };

    bus.connected = VCAN_MAX_CONNECTED_NODES;
    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_TOO_MANY_CONNECTED);
    atto_eq(bus.connected, VCAN_MAX_CONNECTED_NODES);
}


static void test_disconnect_null_bus(void)
{
    vcan_err_t err = vcan_disconnect(NULL, NULL);

    atto_eq(err, VCAN_NULL_BUS);
}

static void test_disconnect_null_node(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);

    err = vcan_disconnect(&bus, NULL);

    atto_eq(err, VCAN_NULL_NODE);
}

static void test_disconnect_valid(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
    };
    err = vcan_connect(&bus, &node);
    atto_eq(err, VCAN_OK);

    err = vcan_disconnect(&bus, &node);

    atto_eq(err, VCAN_OK);
    atto_eq(bus.connected, 0);
}

static void test_disconnect_empty_bus(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
    };

    err = vcan_disconnect(&bus, &node);

    atto_eq(err, VCAN_NODE_NOT_FOUND);
    atto_eq(bus.connected, 0);
}

static void test_disconnect_not_found(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node_1 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .id = 1
    };
    vcan_node_t node_2 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .id = 2
    };
    err = vcan_connect(&bus, &node_1);
    atto_eq(err, VCAN_OK);

    err = vcan_disconnect(&bus, &node_2);

    atto_eq(err, VCAN_NODE_NOT_FOUND);
    atto_eq(bus.connected, 1);
    atto_eq(bus.nodes[0], &node_1);
}

static void test_tx_null_bus(void)
{
    vcan_err_t err = vcan_tx(NULL, NULL, NULL);

    atto_eq(err, VCAN_NULL_BUS);
}

static void test_tx_null_msg(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);

    err = vcan_tx(&bus, NULL, NULL);

    atto_eq(err, VCAN_NULL_MSG);
}


static void test_tx_no_nodes_connected(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_msg_t msg = {
            .id = 20,
            .len = 3,
            .data = {1, 2, 3}
    };

    err = vcan_tx(&bus, &msg, NULL);

    atto_eq(err, VCAN_OK);
    atto_memeq(&msg, &bus.received_msg, sizeof(vcan_msg_t));
}

static void sets_custom_data_to_1(vcan_node_t* node, const vcan_msg_t* msg)
{
    (void) msg;
    node->other_custom_data = (void*) 1;
}

static void sets_custom_data_to_2(vcan_node_t* node, const vcan_msg_t* msg)
{
    (void) msg;
    node->other_custom_data = (void*) 2;
}

static void test_tx_to_all_nodes(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node_1 = {
            .callback_on_rx = sets_custom_data_to_1,
            .other_custom_data = NULL,
            .id = 1,
    };
    vcan_node_t node_2 = {
            .callback_on_rx = sets_custom_data_to_1,
            .other_custom_data = NULL,
            .id = 2,
    };
    err = vcan_connect(&bus, &node_1);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node_2);
    atto_eq(err, VCAN_OK);
    // Setting one callback after connecting.
    node_2.callback_on_rx = sets_custom_data_to_2;
    vcan_msg_t msg = {
            .id = 20,
            .len = 3,
            .data = {1, 2, 3}
    };

    err = vcan_tx(&bus, &msg, NULL);

    atto_eq(err, VCAN_OK);
    // Message was copied
    atto_memeq(&msg, &bus.received_msg, sizeof(vcan_msg_t));
    // Callbacks were called
    atto_eq((int) node_1.other_custom_data, 1);
    atto_eq((int) node_2.other_custom_data, 2);
}


static void test_tx_to_all_nodes_except_source(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node_1 = {
            .callback_on_rx = sets_custom_data_to_1,
            .other_custom_data = NULL,
            .id = 1
    };
    vcan_node_t node_2 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .id = 2
    };
    err = vcan_connect(&bus, &node_1);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node_2);
    atto_eq(err, VCAN_OK);
    // Setting one callback after connecting.
    node_2.callback_on_rx = sets_custom_data_to_2;
    vcan_msg_t msg = {
            .id = 20,
            .len = 3,
            .data = {1, 2, 3}
    };

    err = vcan_tx(&bus, &msg, &node_2);

    atto_eq(err, VCAN_OK);
    // Message was copied
    atto_memeq(&msg, &bus.received_msg, sizeof(vcan_msg_t));
    // Callback was called
    atto_eq((int) node_1.other_custom_data, 1);
    // Source node untouched
    atto_eq(node_2.other_custom_data, NULL);
}

static void callback_print_msg(vcan_node_t* node, const vcan_msg_t* msg)
{
    printf("Node %"PRIu32" received "
           "ID: 0x%08"PRIX32""
           " | Len: %"PRIu32""
           " | Data: ",
           node->id,
           msg->id,
           msg->len);
    for (size_t i = 0; i < msg->len; i++)
    {
        printf("%02"PRIX8" ", msg->data[i]);
    }
    puts("");
}

static void test_readme_example(void)
{
    // Create and initialise a virtual bus.
    vcan_bus_t bus;
    vcan_err_t err;
    err = vcan_init(&bus);  // Fails on NULL args
    assert(err == VCAN_OK);

    // Create 3 virtual nodes.
    // Make them print the received message on reception.
    vcan_node_t node_1 = {
            .id = 1,
            .callback_on_rx = callback_print_msg,
    };
    vcan_node_t node_2 = {
            .id = 2,
            .callback_on_rx = callback_print_msg,
    };
    vcan_node_t node_3 = {
            .id = 3,
            .callback_on_rx = callback_print_msg,
    };

    // Connect the nodes to the bus - by default up to 16 nodes.
    // It's just a #define constant, it can be changed easily.
    err = vcan_connect(&bus, &node_1); // Fails on NULL args or full bus
    assert(err == VCAN_OK);
    err = vcan_connect(&bus, &node_2); // Fails on NULL args or full bus
    assert(err == VCAN_OK);
    err = vcan_connect(&bus, &node_3); // Fails on NULL args or full bus
    assert(err == VCAN_OK);

    // Transmit!
    puts("Transmitting from node 1, node 2 and 3 receive.");
    const vcan_msg_t msg = {
            .id = 0xABCD,
            .len = 3,
            .data = {0x00, 0x1A, 0x2B}
    };
    err = vcan_tx(&bus, &msg, &node_1); // Fails on NULL bus or NULL msg
    assert(err == VCAN_OK);

    // Because we transmitted from node 1, only node 2 and node 3 received the
    // message. Of course node 1 did not receive it, because it sent it.
    // The callbacks were triggered immediately and your stdout should
    // look like this:
    //
    // Transmitting from node 1, node 2 and 3 receive.
    // Node 2 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B
    // Node 3 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B

    // If you transmit from a NULL node, then all nodes receive the message.
    puts("Transmitting from NULL node, everyone receives.");
    err = vcan_tx(&bus, &msg, NULL);
    assert(err == VCAN_OK);

    // And now the stdout should look like this:
    //
    // Transmitting from NULL node, everyone receives.
    // Node 1 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B
    // Node 2 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B
    // Node 3 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B

    // Disconnecting nodes is easy.
    err = vcan_disconnect(&bus, &node_2); // Fails on NULL args or already
    // disconnected node
    assert(err == VCAN_OK);

    // Now node_2 will not receive anything anymore.
    puts("Transmitting from node 1 after node 2 disconnection,"
         " node 3 receives.");
    err = vcan_tx(&bus, &msg, &node_1);
    assert(err == VCAN_OK);

    // And now the stdout should look like this:
    //
    // Transmitting from node 1 after node 2 disconnection, node 3 receives.
    // Node 3 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B
}

int main(void)
{
    test_init_null();
    test_init_valid();
    test_connect_null_bus();
    test_connect_null_node();
    test_connect_null_callback();
    test_connect_valid();
    test_connect_max_reached();
    test_connect_already_connected();
    test_disconnect_null_bus();
    test_disconnect_null_node();
    test_disconnect_empty_bus();
    test_disconnect_not_found();
    test_disconnect_valid();
    test_tx_null_bus();
    test_tx_null_msg();
    test_tx_no_nodes_connected();
    test_tx_to_all_nodes();
    test_tx_to_all_nodes_except_source();
    test_readme_example();
    return atto_at_least_one_fail;
}
