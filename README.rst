Tinypy
======

.. image:: https://github.com/rainwoodman/tinypy/workflows/main/badge.svg
    :target: https://github.com/rainwoodman/tinypy/workflows/main

This is a refactored version of tinypy.

The focus is on the core language, and production readiness features
(testing, code abstraction).

The work is in-progress. Expect bugs.


See README.txt.old for a description of the original tinypy.

build
-----

To build this version of tinypy, you will need a Python (2/3) to bootstrap
the compiler.

.. code::

    make

To run the (incomplete) test suite:

.. code::

    make test

    # stop on first failure
    make xtest

    # run full gc every step.
    make test-dbg

run
---

To run a script:

.. code::

    ./tpy tests/test_str.py


To see the disassembly:

.. code::

    ./tpc -d tests/test_str.py

execution pipeline
------------------

Compiler:
source code -> tokenize -> parse -> encode -> tpc

VM:
tpc -> tpvm

A module can be imported only if it is already embedded.

'site' modules are in modules/ (may be broken as there are no test coverages).

Internal runtime modules (part of the language) written in tpy are in tinypy/runtime.

namespaces
----------

tinypy.compiler: the compiler (`compiler/`)

tinypy.runtime: the runtime support (`runtime/*.py`)

tinypy.runtime.builtins: the runtime in C (`tpy_builtins.c`).

Of course things may still need to be shuffled around between these modules


