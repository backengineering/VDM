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

	if (!result) 
		return { {}, {} };

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