/*
 * InTCPtor - a library to simulate network trouble by intercepting socket calls
 *
 * This file contains the startup guard class to resolve original functions on startup and set up the library.
 */

#pragma once

// guard class to resolve original functions on startup
class CStartup_Guard final {
    public:
        CStartup_Guard();
        ~CStartup_Guard();
};

extern CStartup_Guard gStartup_Guard;
