# Vulnerable Driver Manipulation

A library to manipulate driver exposing a physical read/write primitive to allow the user to call any function in the kernel. There are thousands of drivers exposing physical
memory read/write, a bunch are listed in this repo.

# Usage

Currently the project is configured to use gdrv, but if you want to swap the driver out you must defined four functions. 