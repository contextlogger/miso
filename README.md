About
-----

This is Miso, a utility library for Python on Series 60, consisting
solely of a single Symbian native DLL. It contains odd utilities that
are not in the PyS60 platform itself (and which would mostly seem out
of place there), and are perhaps too small to warrant maintaining in
their own library.

Homepage
--------

http://contextlogger.org/miso/

Contents
--------

The material to be found in this source distribution includes:

  * build/ -- standard Symbian build files, i.e. bld.inf, .mmp, 
    and .pkg files, for building a SIS file from source (see the
    documentation of a Symbian SDK for more info).

  * private-cxx-api/ -- API documentation for the C++ source code.
    Given that this module is not intended to have a public C++ API
    (just a public Python API), the target audience for this is Miso
    developers. Generated using Doxygen. doxygen makefile (called
    doxyfile) is also provided.

  * python-api/ -- API documentation for this Python module. Generated
    using epydoc from miso.py, which describes the Python interface of
    this component (just the interface, not the implementation). If
    changing the API provided by the native Python library, please
    also modify miso.py accordingly.

  * README.md -- this file.

  * src/ -- C++ source for the native library.

  * test-programs/ -- some test code that also, at least in some
    cases, provide decent examples on how to use the library.

  * web/ -- website material; also contains the license covering
    this software.

Building
--------

The library should build with a number of different C++ compilers
against a number of different S60 system API versions, provided that
compatible makefiles (defining the appropriate preprocessor macros,
etc.) are chosen.

The Symbian (and GnuPoc) style makefiles have been generated using the
provided top-level makefile (sakefile.rb).

Contributions
-------------

Contributions to Miso are welcome. If you have a snippet of Symbian
code that you would like to make available from Python, consider
contributing it to Miso. That way you can avoid the hassle of building
and releasing PYD/SIS files for a number of different (current and
future) S60 versions.
