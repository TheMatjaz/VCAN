/**
 * @file
 *
 * Unit test suite.
 *
 * @copyright Copyright © 2020, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 * @license BSD 3-clause license.
 */

#include "atto.h"
#include "vcan.h"

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
    atto_zeros((uint8_t*) &bus, sizeof(bus));
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
            .received_msg.id = 42
    };

    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_NULL_CALLBACK);
}

static void does_nothing(vcan_node_t* node)
{
}

static void test_connect_valid(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42
    };

    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_OK);
}


static void test_connect_already_connected(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42
    };

    err = vcan_connect(&bus, &node);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_ALREADY_CONNECTED);
}

static void test_connect_max_reached(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42
    };

    bus.connected = VCAN_MAX_CONNECTED_NODES;
    err = vcan_connect(&bus, &node);

    atto_eq(err, VCAN_TOO_MANY_CONNECTED);
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
            .received_msg.id = 42
    };
    err = vcan_connect(&bus, &node);
    atto_eq(err, VCAN_OK);

    err = vcan_disconnect(&bus, &node);

    atto_eq(err, VCAN_OK);
}

static void test_disconnect_empty_bus(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42
    };

    err = vcan_disconnect(&bus, &node);

    atto_eq(err, VCAN_NODE_NOT_FOUND);
}

static void test_disconnect_not_found(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42
    };
    vcan_node_t node2 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 42 + 1
    };
    err = vcan_connect(&bus, &node);
    atto_eq(err, VCAN_OK);

    err = vcan_disconnect(&bus, &node2);

    atto_eq(err, VCAN_NODE_NOT_FOUND);
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
}

static void sets_custom_data_to_ff(vcan_node_t* node)
{
    node->other_custom_data = (void*) 0xFF;
}

static void test_tx_to_all_nodes(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node1 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = sets_custom_data_to_ff,
            .received_msg.id = 42,
            .received_msg.len = 50
    };
    vcan_node_t node2 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 43,
            .received_msg.len = 51
    };
    err = vcan_connect(&bus, &node1);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node2);
    atto_eq(err, VCAN_OK);
    // Setting one callback after connecting.
    node2.callback_on_rx = sets_custom_data_to_ff;
    vcan_msg_t msg = {
            .id = 20,
            .len = 3,
            .data = {1, 2, 3}
    };

    err = vcan_tx(&bus, &msg, NULL);

    atto_eq(err, VCAN_OK);
    // Message was copied
    atto_memeq(&node1.received_msg, &msg, sizeof(msg));
    atto_memeq(&node2.received_msg, &msg, sizeof(msg));
    // Callbacks were called
    atto_eq(node1.other_custom_data, (void*) 0xFF);
    atto_eq(node2.other_custom_data, (void*) 0xFF);
}


static void test_tx_to_all_nodes_except_source(void)
{
    vcan_bus_t bus;
    vcan_err_t err = vcan_init(&bus);
    atto_eq(err, VCAN_OK);
    vcan_node_t node1 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = sets_custom_data_to_ff,
            .received_msg.id = 42,
            .received_msg.len = 50
    };
    vcan_node_t node2 = {
            .callback_on_rx = does_nothing,
            .other_custom_data = NULL,
            .received_msg.id = 43,
            .received_msg.len = 51
    };
    err = vcan_connect(&bus, &node1);
    atto_eq(err, VCAN_OK);
    err = vcan_connect(&bus, &node2);
    atto_eq(err, VCAN_OK);
    // Setting one callback after connecting.
    node2.callback_on_rx = sets_custom_data_to_ff;
    vcan_msg_t msg = {
            .id = 20,
            .len = 3,
            .data = {1, 2, 3}
    };

    err = vcan_tx(&bus, &msg, &node2);

    atto_eq(err, VCAN_OK);
    // Message was copied
    atto_memeq(&node1.received_msg, &msg, sizeof(msg));
    // Callbacks were called
    atto_eq(node1.other_custom_data, (void*) 0xFF);
    // Source node untouched
    atto_eq(node2.other_custom_data, NULL);
    atto_eq(node2.received_msg.len, 51);
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
    return atto_at_least_one_fail;
}
