# Credits

Before I begin, those who helped me create this project shall be credited.

- [Can1357](https://blog.can.ac), for helping me find the correct page in physical memory.
- Ch40zz, for helping me fix many issues in things I could never have fixed.
- wlan, I used your drv_image class :)

# Physmeme

Given ANY map/unmap (read/write) of physical memory, one can now systematically map unsigned code into ones kernel.
Many drivers expose this primitive and now can all be exploited by simply coding a few functions. 

### What drivers support physical read/write?

Any driver exposing MmMapIoSpace/MmUnmapIoSpace or ZwMapViewOfSection/ZwUnmapViewOfSection can be exploited. This means bios flashing utils, fan speed utils
(like MSI Afterburner), or general windows system utilities that expose physical read/write. 

If you are in any sort of doubt about the abundance of these drivers simply go to 
<a href="https://www.unknowncheats.me/forum/anti-cheat-bypass/334557-vulnerable-driver-megathread.html">this</a> page and ctrl-f "MmMapIoSpace". (24 results)

### How does this exploit work?

Since we are able to read/write to any physical memory on the system the goal is to find the physical page of a syscall. This can be done by calculating the offset into the page in which the syscall resides. Doing so is trivial and only requires the modulo operation.

```cpp
auto syscall_page_offet = rva % 0x1000;
```

Now that we know that the syscalls bytes are going to be that far into the physical page we can map each physical page into our process 512 at a time (2mb) and then
check the page + page_offset and compare with the syscalls bytes. After we have the syscalls page we can install inline hooks and then call into the function.

<img src="https://cdn.discordapp.com/attachments/687446832175251502/701355063939039292/unknown.png"/>

### How long does it take to find the physical page?

Less then one second. For each physical memory range I create a thread that maps 2mb at a time of physical memory and scans each physical page. This is on a system with 16gb.

In other words... its very fast, you wont need to worry about waiting to find the correct page.

# DriverEntry

you can change the paremeters you pass to driver entry simply by changing this:

```cpp
using DRIVER_INITIALIZE = NTSTATUS(__stdcall*)(std::uintptr_t, std::size_t);
```

right now your entry point should look like this:

```cpp
NTSTATUS DriverEntry(PVOID lpBaseAddress, DWORD32 dwSize)
```

The source the hello-world.sys is the following:

```cpp
#include <ntifs.h>

NTSTATUS DriverEntry(PVOID lpBaseAddress, DWORD32 dwSize)
{
	DbgPrint("> Base Address: 0x%p, Size: 0x%x", lpBaseAddress, dwSize);
	return STATUS_SUCCESS;
}

```