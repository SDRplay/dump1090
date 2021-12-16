# DUMP1090

The latest version number of this particular build of `dump1090` is `1.47`

This build supports all RSPs and requires API `3.x` which can be downloaded from the `SDRplay`

Downloads page: https://www.sdrplay.com/downloads

This build also still supports the `RTL-SDR` device and so the `RTL-SDR` library needs to be installed also.

## Build from source

    $ git clone https://github.com/SDRplay/dump1090<br />
    $ cd dump1090

NOTE: BEFORE BUILDING CHECK PATHS IN THE Makefile...

PKG_CONFIG_PATH: add the path to the librtlsdr.pc file<br />
SDRPLAY_CFLAGS: If you have problems including the API header files, add the path to the API inc directory here<br />
SDRPLAY_LIBS: If you have problems with linking to the API dll, add the path here<br />

There are examples of the SDRPlay variables if building on Cygwin,n the Makefile. They may not be required if building on Linux

    $ SDRPLAY=1 make dump1090

To build on Windows, use Cygwin - note: after building under Cygwin you may need to specify the path to the API dll,
or add the API dll path to your system path. An example of how to do this on the command line is...

    $ PATH="/cygdrive/c/program files/sdrplay/api/x86":$PATH ./dump1090.exe

## Usage example 

You should specify SDRPlay at start:

    $ ./dump1090 --dev-sdrplay


-------------------------------------------------

This is a fork of mutability's version of dump1090 (https://github.com/mutability/dump1090)

This version is licensed under the GPL (v2 or later). See the file COPYING for details.
