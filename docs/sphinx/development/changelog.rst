Changelog
*********

.. contents:: Releases:
   :local:
   :depth: 1

..
    Features
    C API
    Bindings
    Command-line tools
    Applications
    Bug fixes
    Portability
    Security
    Internals
    Build system
    Packaging
    Tests
    Documentation

Version 0.2.5 (Jul 28, 2023)
============================

Bug fixes
---------

* fix byte order conversion

Build system
------------

* fix compiler type detection when compiler is specified via ``CC`` or ``CXX`` variable
* export symbols of dependencies built by ``--build-3rdparty`` when building static library (``libroc.a``), to avoid linker errors when using it

Version 0.2.4 (May 13, 2023)
============================

C API
-----

* always set ``file`` and ``line`` in ``roc_log_message``

Command-line tools
------------------

* support PulseAudio sources in ``roc-send``
* support ``--io-latency`` option in ``roc-send``

Bug fixes
---------

* fix potential race
* fix byte order detection on Android
* do not write to log from shared library destructor
* stop using user-provided log handler after entering shared library destructor

Internals
---------

* improve logging
* refactor scons scripts

Build system
------------

* fix ``--build-3rdparty=sox`` when ``sndio`` is installed
* fix ``--build-3rdparty=google-benchmark`` when there is ``python3``, but no ``python`` in PATH
* fix OpenSSL platform detection in ``--build-3rdparty=openssl`` when not cross-compiling
* set Android API level to ``21``
* add ``--macos-platform`` and ``--macos-arch`` scons options
* by default, set ``--macos-platform`` to current OS, to avoid linker warnings about incompatible macOS deployment targets
* support building macOS universal binaries by providing multiple values for ``--macos-arch``
* propagate Android platform, macOS platform, and macOS architectures to ``--build--3rdparty``
* unexport all symbols except ``roc_*`` from ``libroc.so`` and ``libroc.a`` on Linux, and ``libroc.dylib`` on macOS
* resolve ``pkg-config`` absolute path

Documentation
-------------

* minor updates

Version 0.2.3 (Mar 9, 2023)
===========================

C API
-----

* add ``roc_receiver_set_reuseaddr`` and ``roc_sender_set_reuseaddr``

Command-line tools
------------------

* add ``--reuseaddr`` to ``roc-recv`` and ``roc-send``

Bug fixes
---------

* fix formatting of endpoint URI with zero port
* fix usage of multicast with RTCP in ``roc-recv``

Build system
------------

* add new dependency OpenSSL
* fix work with SCons 4.5
* exclude sox and libpulse from .pc file for libroc

Packaging
---------

* add debian packages and publish them on github
* add rpm packages spec

Documentation
-------------

* minor updates

Version 0.2.2 (Feb 27, 2023)
============================

C API
-----

* rename ``roc_get_version`` to ``roc_version_get``

Bug fixes
---------

* fix crash in ``roc_log_set_handler`` when argument is NULL

Build system
------------

* fix build on recent Android NDK
* install ``.pc`` file to ``<libdir>/pkgconfig`` instead of ``PKG_CONFIG_PATH``
* add support for ``DESTDIR``
* strip symbols in release build

Documentation
-------------

* minor updates

Version 0.2.1 (Dec 26, 2022)
============================

Build system
------------

* install to ``/usr`` by default (except macOS)

Documentation
-------------

* minor updates

Version 0.2.0 (Dec 19, 2022)
============================

Features
--------

* support multicast
* support broadcast
* support speex resampler and make it default
* support slots (connect sender to multiple receivers and vice versa)
* initial support for RTCP

C API
-----

* return error codes from ``roc_context_open``, ``roc_receiver_open``, ``roc_sender_open``
* introduce ``roc_endpoint`` to identify endpoints using URI
* rename ``roc_fec_code`` to ``roc_fec_encoding``
* add ``roc_resampler_backend``
* add ``roc_clock_source``
* add ``roc_version`` and friends

Bindings
--------

* add Go bindings (`roc-go <https://github.com/roc-streaming/roc-go/>`_)
* add Java bindings (`roc-java <https://github.com/roc-streaming/roc-java/>`_)

Command-line tools
------------------

* use URIs to identify audio devices and endpoints
* add ``--backup`` option to ``roc-recv``
* replace ``--frame-size`` with ``--frame-length`` and ``--frame-limit``
* remove ``--resampler-interp`` and ``--resampler-window``

Applications
------------

* move PulseAudio modules to `roc-pulse <https://github.com/roc-streaming/roc-pulse/>`_ repo
* add `roc-droid <https://github.com/roc-streaming/roc-droid/>`_ Android app

Bug fixes
---------

* fix race in PRNG
* fix race in mutex and semaphore on macOS
* fix potential deadlock in network code

Portability
-----------

* Linux / aarch64 build fixes
* Android build fixes
* macOS build fixes
* FreeBSD build fixes
* support generic Unix target
* continuous integration for more Linux distros
* continuous integration for Android
* testing on Raspberry Pi 4

Internals
---------

* add ``roc_peer`` module
* add ``roc_ctl`` module
* support for asynchronous tasks in ``roc_pipeline``, ``roc_netio``, ``roc_ctl``
* lock-free task queues
* optimizations to avoid unnecessary context switches
* improvements in memory pools
* improvements in logger
* self-profiling
* start work on SDP support
* preparations for RTSP support
* rework project structure
* lots of small improvements

Build system
------------

* add ``--enable-static`` and ``--disable-shared``
* add ``--disable-soversion`` option
* compatibility with recent SCons versions
* compatibility with different Python versions
* improve toolchain detection
* generate ``.pc`` file for pkg-config
* fix build with recent PulseAudio
* fix build with recent libunwind
* fixes for building third-parties

Tests
-----

* add benchmarks
* lots of small updates

Documentation
-------------

* document Android bulding and testing
* lots of small updates

Version 0.1.5 (Apr 5, 2020)
===========================

Portability
-----------

* fix building on Manjaro Linux
* fix building on Yocto Linux
* add openSUSE to continuous integration and user cookbook
* drop Xcode 7.3 from continuous integration, add Xcode 11.3

Build system
------------

* correctly handle arguments in environment variables like CXX/CC/LD/etc (for Yocto Linux)
* correctly handle spaces in environment variables (for Yocto Linux)
* fix environment overrides checks
* fix building of the host tools when cross-compiling
* fix warnings on Clang 11
* fix sphinx invocation
* explicitly disable Orc when building PulseAudio using --build-3rdparty
* explicitly enable -pthread or -lpthread for libsndfile (for Manjaro Linux)
* user CMake instead of autotools when building libuv for Android using ``--build-3rdparty``
* switch to libuv 1.35.0 by default in ``--build-3rdparty``
* check for unknown names in ``--build-3rdparty``

Version 0.1.4 (Feb 6, 2020)
===========================

Internals
---------

* fix logging

Build system
------------

* make ``/usr/local`` prefix default everywhere except Linux
* make default compiler consistent with CXX var
* fix handling of RAGEL, GENGETOPT, DOXYGEN, SPHINX_BUILD, and BREATHE_APIDOC vars
* fix SoX download URL (again)
* fix CPU count calculation

Documentation
-------------

* update PulseAudio version numbers in "User cookbook"
* update CONTRIBUTING and "Coding guidelines"
* update maintainers and contributors list

Version 0.1.3 (Oct 21, 2019)
============================

Command-line tools
------------------

* add ``--list-drivers`` option
* add git commit hash to version info

Internals
---------

* print backtrace on Linux and macOS using libunwind instead of glibc backtrace module
* print backtrace on Android using bionic backtrace module
* colored logging

Build system
------------

* add libunwind optional dependency (enabled by default)
* add ragel required dependency
* rename "uv" to "libuv" in ``--build-3rdparty``
* don't hide symbols in debug builds
* strip symbols in release builds
* fix building on recent Python versions
* fix SoX download URL
* fix PulseAudio version parsing
* automatically apply memfd patch when building PulseAudio
* automatically fix libasound includes when building PulseAudio

Version 0.1.2 (Aug 14, 2019)
============================

Bug fixes
---------

* fix handling of inconsistent port protocols / FEC schemes
* fix IPv6 support
* fix incorrect usage of SO_REUSEADDR
* fix panic on bind error
* fix race in port removing code
* fix packet flushing mechanism
* fix backtrace printing on release builds

Portability
-----------

* fix building on musl libc
* continuous integration for Alpine Linux

Internals
---------

* rework audio codecs interfaces (preparations for Opus and read-aheads support)
* minor refactoring in FEC support
* improve logging

Build system
------------

* allow to configure installation directories
* auto-detect system library directory and PulseAudio module directory

Documentation
-------------

* extend "Forward Erasure Correction codes" page
* add new pages: "Usage", "Publications", "Licensing", "Contacts", "Authors"
* replace "Guidelines" page with "Contribution Guidelines", "Coding guidelines", and "Version control"

Version 0.1.1 (Jun 18, 2019)
============================

Bug fixes
---------

* fix memory corruption in OpenFEC / LDPC-Staircase (fix available in our fork)
* fix false positives in stream breakage detection

Portability
-----------

* start working on Android port; Roc PulseAudio modules are now available in Termux unstable repo
* continuous integration for Android / arm64 (minimal build)
* docker image for aarch64-linux-android toolchain

Build system
------------

* fix multiple build issues on macOS
* fix multiple build issues with cross-compilation and Android build
* fix issues with building third-parties
* fix issues with compilation db generation
* set library soname/install_name and install proper symlinks
* improve configuration options
* improve system type detection and system tools search
* improve scripts portability
* better handling of build environment variables

Tests
-----

* fix resampler AWGN tests
* add travis job to run tests under valgrind

Version 0.1.0 (May 28, 2019)
============================

Features
--------

* streaming CD-quality audio using RTP (PCM 16-bit stereo)
* maintaining pre-configured target latency
* restoring lost packets using FECFRAME with Reed-Solomon and LDPC-Staircase FEC schemes
* converting between the sender and receiver clock domains using resampler
* converting between the network and input/output sample rates
* configurable resampler profiles for different CPU and quality requirements
* mixing simultaneous streams from multiple senders on the receiver
* binding receiver to multiple ports with different protocols
* interleaving packets to increase the chances of successful loss recovery
* detecting and restarting broken streams

C API
-----

* initial version of transport API (roc_sender, roc_receiver)

Command-line tools
------------------

* initial version of command-line tools (roc-send, roc-recv, roc-conv)

Applications
------------

* initial version of PulseAudio transport (module-roc-sink, module-roc-sink-input)

Portability
-----------

* GNU/Linux support
* macOS support
* continuous integration for Ubuntu, Debian, Fedora, CentOS, Arch Linux, macOS
* continuous integration for x86_64, ARMv6, ARMv7, ARMv8
* toolchain docker images for arm-bcm2708hardfp-linux-gnueabi, arm-linux-gnueabihf, aarch64-linux-gnu
* testing on Raspberry Pi 3 Model B, Raspberry Pi Zero W, Orange Pi Lite 2
