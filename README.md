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