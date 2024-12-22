# InTCPtor

The InTCPtor (pronounced: interceptor) is a library that intercepts TCP-related BSD socket function calls and introduces delays, message fragmentation and more.

This software is developed for KIV/UPS (Fundamentals of Computer Networks, CZ: Úvod do počítačových sítí) semestral work evaluation process. 

Please note, that this software works only on GNU/Linux as it uses a platform-dependent way to override standard library calls. There is a chance, that in future, we will enhance the software to support macOS, but it is highly unlikely MS Windows will be supported.

There are two main components:
- `libintcptor-overrides` - dynamic library (shared object) that implements the overrides
- `intcptor-run` - runner that sets the `LD_PRELOAD` environment variable and starts the executable

## Build

To build the library and runner, you can execute the following commands:

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

If you intend to use the runner to start your network application, you can do it without any additional hassle. The only requirement is to have the runner and overrides library in the same directory:

```
./intcptor-run my-server 127.0.0.1 10000
```

The `intcptor-run` executable sets the environment variable `LD_PRELOAD` and starts the desired executable.

Alternatively, you can invoke the executable with the overrides library without the runner by setting the `LD_PRELOAD` manually:

```
LD_PRELOAD=./libintcptor-overrides.so ./my-server 127.0.0.1 10000
```

## What does it do?

It hooks the following functions: `socket`, `close`, `accept`, `recv`, `read`, `send`, `write`.

For now, the `read()` call upon a managed socket is translated as a call to `recv()` with `flags` parameter set to zero. The same applies to `write()` and `send()`.

Intercepted `recv()`:
* 30 % chance to pass-through without modifications
* 10 % chance to read 1 byte less than requested
* 30 % chance to read half of what is expected
* 20 % chance to read only 2 bytes
* 10 % chance to read 2 bytes less that requested

Intercepted `send()`:
* 30 % chance to pass-through without modifications
* 10 % chance to send the message 1 byte after byte with delays between each `send()`
* 30 % chance to send message split in half (two `send()` calls) with delays between each `send()`
* 20 % chance to send first 2 bytes and then send the rest of the message with delays between each `send()`
* 10 % chance to send the message in 2B chunks with delays between each `send()`

The delay is calculated as `100 + rand * 300`, where `rand` is a floating point number from 0 to 1.

## Planned features

* make the interceptions non-blocking (the sender is now blocked when introducing delays; we should introduce an output queue to slow down the traffic in a separate thread)
    * following this feature, we may be more creative regarding the trouble (e.g., we can join two consecutive messages to one `send()`)
* UDP support
    * same base features as for TCP
    * random whole message dropping
    * dropping a part of message
    * random message byte reordering
* learning the PDU format
    * e.g., if it contains a magic identifier, if there's always a terminating character, etc.
    * when the PDU format is known, allow creating a valid PDU at random times (or when requested)
* `sendmsg()` and `recvmsg()` support
* configuration (e.g., the chances)
* user control (e.g., controllably close sockets, send user-defined message, ...)
* randomly dropping TCP connections as a result of a simulated network disruption
* statistics

## License

The InTCPtor software is distributed under MIT license. See attached LICENSE file for full licencing information.

|![University of West Bohemia](https://www.zcu.cz/en/assets/logo.svg)|![Department of Computer Science and Engineering](https://www.kiv.zcu.cz/site/documents/verejne/katedra/dokumenty/dcse-logo-barevne.png)|
|--|--|