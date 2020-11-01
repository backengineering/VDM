#pragma once
#include <windows.h>
#include <string_view>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include "../vdm/vdm.hpp"

namespace vdm
{
	constexpr std::pair<const char*, const char*> syscall_hook = { "NtShutdownSystem", "ntdll.dll" };
	inline std::atomic<bool> is_page_found = false;
	inline std::atomic<void*> syscall_address = nullptr;

	inline std::uint16_t nt_page_offset;
	inline std::uint32_t nt_rva;
	inline std::uint8_t* ntoskrnl_buffer;

	class vdm_ctx
	{
	public:
		vdm_ctx();
		template <class T, class ... Ts>
		__forceinline std::invoke_result_t<T, Ts...> syscall(void* addr, Ts ... args) const
		{
			static const auto proc = 
				GetProcAddress(
					GetModuleHandleA(syscall_hook.second),
					syscall_hook.first
				);

			static std::mutex syscall_mutex;
			syscall_mutex.lock();

			// jmp [rip+0x0]
			std::uint8_t jmp_code[] =
			{
				0xff, 0x25, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00
			};

			std::uint8_t orig_bytes[sizeof jmp_code];
			*reinterpret_cast<void**>(jmp_code + 6) = addr;
			vdm::read_phys(vdm::syscall_address.load(), orig_bytes, sizeof orig_bytes);

			// execute hook...
			vdm::write_phys(vdm::syscall_address.load(), jmp_code, sizeof jmp_code);
			auto result = reinterpret_cast<T>(proc)(args ...);
			vdm::write_phys(vdm::syscall_address.load(), orig_bytes, sizeof orig_bytes);

			syscall_mutex.unlock();
			return result;
		}
	private:
		void locate_syscall(std::uintptr_t begin, std::uintptr_t end) const;
		bool valid_syscall(void* syscall_addr) const;
	};
}