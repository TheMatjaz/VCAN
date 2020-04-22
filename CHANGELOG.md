Changelog
===============================================================================

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

*******************************************************************************

[2.0.0] - 2020-04-22
----------------------------------------

Performance improvement, which breaks compatibility, although slightly.


### Modified

- The message is not copied into the node anymore, it is removed from the 
  `vcan_node_t` struct. Instead it is copied once and only once into the
  `vcan_bus_t` struct, to avoid copying it into every single node.
- Due to the upper point, the callbacks now obtain also a const pointer to the
  message as a second parameter.


### Fixed

- Minor doxygen documentation improvements.



[1.0.0] - 2020-04-21
----------------------------------------

Initial version.


### Added

- Connect/disconnect virtual nodes to/from bus.
- Transmit message on bus, every node gets a copy and its callback is called.
