<div align="center">
    <div>
        <img src="https://imgur.com/7ipSZRN.png"/>
    </div>
</div>

# Vulnerable Driver Manipulation

A library to manipulate drivers exposing a physical memory read/write primitive to allow the user to call any function in the kernel. There are thousands of drivers exposing physical
memory read/write, a bunch are listed in this repo. Currently the project is using gdrv.sys, and is inline hooking NtShutdownSystem. The inline hook is not patchguard friendly,
but is removed after every syscall into NtShutdownSystem to prevent possible detection. 

# Usage

Currently the project is configured to use gdrv, but if you want to swap the driver out you must defined four functions. 