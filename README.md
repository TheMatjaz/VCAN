VCAN is a tiny virtual CAN and CAN-FD bus in C
===============================================================================

Especially useful for debugging and testing without using actual
CAN-connected devices, VCAN is a tiny C library that allows the user to connect
virtual nodes on a virtual bus and make them react whenever someone transmits a
message on the bus.

After the transmission, each node obtains a copy of the message
and a callback on each node is called to warn the node of a message being
received.



Known limitations
----------------------------------------

VCAN is simple, synchronous and single-threaded. It does not simulate
transmission errors, collisions, arbitration, etc. just pure data transfer.
Callbacks should be fast.

... but you are free to alter it to your specific needs!



Usage example
----------------------------------------

```c
// Create and initialise a virtual bus.
vcan_bus_t bus;
vcan_err_t err;
err = vcan_init(&bus);  // Fails on NULL args
assert(err == VCAN_OK);

// Create 3 virtual nodes.
// Make them print the received message on reception using the callback.
// The definition of callback_print_msg() is available in the tst/test.c file
vcan_node_t node1 = {
        .id = 1,
        .callback_on_rx = callback_print_msg,
};
vcan_node_t node2 = {
        .id = 2,
        .callback_on_rx = callback_print_msg,
};
vcan_node_t node3 = {
        .id = 3,
        .callback_on_rx = callback_print_msg,
};

// Connect the nodes to the bus - by default up to 16 nodes.
// It's just a #define constant, it can be changed easily.
err = vcan_connect(&bus, &node1); // Fails on NULL args or full bus
assert(err == VCAN_OK);
err = vcan_connect(&bus, &node2); // Fails on NULL args or full bus
assert(err == VCAN_OK);
err = vcan_connect(&bus, &node3); // Fails on NULL args or full bus
assert(err == VCAN_OK);

// Transmit!
puts("Transmitting from node 1, node 2 and 3 receive.");
const vcan_msg_t msg = {
        .id = 0xABCD,
        .len = 3,
        .data = {0x00, 0x1A, 0x2B}
};
err = vcan_tx(&bus, &msg, &node1); // Fails on NULL bus or NULL msg
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
err = vcan_disconnect(&bus, &node2); // Fails on NULL args or already
// disconnected node
assert(err == VCAN_OK);

// Now node2 will not receive anything anymore.
puts("Transmitting from node 1 after node 2 disconnection,"
     " node 3 receives.");
err = vcan_tx(&bus, &msg, &node1);
assert(err == VCAN_OK);

// And now the stdout should look like this:
//
// Transmitting from node 1 after node 2 disconnection, node 3 receives.
// Node 3 received ID: 0x0000ABCD | Len: 3 | Data: 00 1A 2B
```

You can also check the `tst/test.c` file for more examples.



Include it in your project
----------------------------------------

### Static source inclusion

Copy the `inc/vcan.h` and `src/vcan.c` files into your existing
C project, add them to the source folders and compile. Done.



### Compiling into all possible targets

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

This will build all targets:

- a `libvcan.a` static library
- a test runner executable `testvcan`
- the Doxygen documentation (if Doxygen is installed)

To compile with the optimisation for size, use the
`-DCMAKE_BUILD_TYPE=MinSizeRel` flag instead.
