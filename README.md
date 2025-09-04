# InTCPtor

![Build](https://github.com/MartinUbl/InTCPtor/actions/workflows/cmake-single-platform.yml/badge.svg)

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

or just invoke the build script, which also automatically sets the `PATH` variable for later use:

```
./build_and_env.sh
```

## Usage

If you intend to use the runner to start your network application, you can do it without any additional hassle. The only requirement is to have the runner and overrides library in the same directory:

```
./intcptor-run my-server 127.0.0.1 10000
```

The `intcptor-run` executable sets the environment variable `LD_PRELOAD` and starts the desired executable. Its use is optional, though convenient.

Alternatively, you can invoke the executable with the overrides library without the runner by setting the `LD_PRELOAD` manually:

```
LD_PRELOAD=./libintcptor-overrides.so ./my-server 127.0.0.1 10000
```

## What does it do?

It hooks the following functions: `socket`, `close`, `accept`, `recv`, `read`, `send`, `write`.

For now, the `read()` call upon a managed socket is translated as a call to `recv()` with `flags` parameter set to zero. The same applies to `write()` and `send()`.

Intercepted `recv()` has, by default, the following properties:
* 30 % chance to pass-through without modifications
* 10 % chance to read 1 byte less than requested
* 30 % chance to read half of what is expected
* 20 % chance to read only 2 bytes
* 10 % chance to read 2 bytes less that requested

Intercepted `send()` has, by default, the following properties:
* 30 % chance to pass-through without modifications
* 10 % chance to send the message 1 byte after byte with delays between each `send()`
* 30 % chance to send message split in half (two `send()` calls) with delays between each `send()`
* 20 % chance to send first 2 bytes and then send the rest of the message with delays between each `send()`
* 10 % chance to send the message in 2B chunks with delays between each `send()`

The delay is calculated according to normal distribution with default mean of 100 and sigma of 10.

## More features

* configuration (e.g., the chances)
* randomly dropping TCP connections as a result of a simulated network disruption

## Configuration

The InTCPtor creates default configuration file named `intcptor_config.cfg` when run for the first time. You can adjust the options as you wish.

|Option|Default value|Description|
|--|--|--|
|`Send__1B_Sends`|0.1|Probability of sending the message split to 1B chunks|
|`Send__2B_Sends`|0.1|Probability of sending the message split to 2B chunks|
|`Send__2_Separate_Sends`|0.3|Probability of splitting the message to two in half|
|`Send__2B_Sends_And_Second_Send`|0.2|Probability of sending the first 2B, and then the rest of the message|
|`Recv__1B_Less`|0.1|Probability of receiving 1 byte less than requested|
|`Recv__2B_Less`|0.1|Probability of receiving 2 bytes less than requested|
|`Recv__Half`|0.3|Probability of receiving half of what was requested|
|`Recv__2B`|0.2|Probability of receiving maximum of 2 bytes|
|`Send_Delay_Ms_Mean`|100|Mean value of artificially added delay to `send()` calls|
|`Send_Delay_Ms_Sigma`|10|Sigma value of artificially added delay to `send()` calls|
|`Drop_Connections`|0|Randomly drop connections (simulate network disruptions)|
|`Drop_Connection_Delay_Ms_Min`|5000|Minimal delay for connection drops|
|`Drop_Connection_Delay_Ms_Max`|15000|Maximal delay for connection drops|
|`Log_Enabled`|1|Is detailed logging enabled?|

## Planned features

* join consecutive `send()` calls through the output queue; for now, there is just a delay and/or tearing a single call to multiple sends
* UDP support
    * same base features as for TCP
    * random whole message dropping
    * dropping a part of message
    * random message byte reordering
* learning the PDU format
    * e.g., if it contains a magic identifier, if there's always a terminating character, etc.
    * when the PDU format is known, allow creating a valid PDU at random times (or when requested)
* `sendmsg()` and `recvmsg()` support
* user control (e.g., controllably close sockets, send user-defined message, ...)
* statistics

## License

The InTCPtor software is distributed under MIT license. See attached LICENSE file for full licencing information.

|![University of West Bohemia](https://www.zcu.cz/en/assets/logo.svg)|![Department of Computer Science and Engineering](https://www.kiv.zcu.cz/site/documents/verejne/katedra/dokumenty/dcse-logo-barevne.png)|
|--|--|