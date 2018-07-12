# libccr

![Travis](https://img.shields.io/travis/Gikoskos/libccr.svg)

Simple and easy-to-use conditional critical regions (CCR) with pthreads in C.

CCR is a high-level synchronization construct with the form

`region R when C do S`

where R is the region's name, C is a condition and S the critical section.

The above statement is translated to

    Enter region R
    Block current thread while condition C is false
    Enter critical section S (when condition C becomes true)
    Execute all code in critical section S
    Leave S
    Leave R

A CCR guarantees mutual exclusion and fairness (no starvation) when checking whether condition C is true or not, and when executing the code in critical section S.


## How to use

Supported platforms are Linux, OSX and Windows (only tested with mingw and libwinpthread), but it can be used anywhere where there's support for pthreads.

The library comes in two flavors: a [macro-only implementation](Doc.md#macro-api) that works simply by including the header file `ccr.h`, and a [regular library](Doc.md#library-api) `ccr.c` that can be compiled statically, and interfaced with, using the library API.

[Documentation and tutorials here](Doc.md).

Various synchronization problems solved with `libccr` can be found on the folder `examples`. These make use of POSIX APIs like `strerror_r`, and VT100 terminal color codes, so they might not compile everywhere.

## License

See LICENSE