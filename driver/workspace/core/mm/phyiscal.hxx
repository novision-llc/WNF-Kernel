#pragma once

namespace mm::phys
{
	static nt_status_t read_direct(std::uint64_t physical_addr, void* buffer, std::size_t size, size_t* bytes = nullptr)
	{
		mm_copy_address_t src{ };
		src.m_physical_address.m_quad_part = physical_addr;

		size_t number_of_bytes = 0;
		return nt::mm_copy_memory(buffer, src, size, copy_physical, (bytes != nullptr) ? bytes : &number_of_bytes);
	}

	static nt_status_t write_direct(std::uint64_t physical_addr, PVOID buffer, size_t size, size_t* bytes = nullptr)
	{
		if (!physical_addr)
			return nt_status_t::unsuccessful;

		physical_address_t AddrToWrite = { 0 };
		AddrToWrite.m_quad_part = physical_addr;

		PVOID pmapped_mem = nt::mm_map_io_space_ex(AddrToWrite, size, PAGE_READWRITE);
		if (!pmapped_mem)
			return nt_status_t::unsuccessful;

		memcpy(pmapped_mem, buffer, size);

		if (bytes != nullptr)
			*bytes = size;

		nt::mm_unmap_io_space(pmapped_mem, size);
		return nt_status_t::success;
	}

    static nt_status_t read_direct_safe(std::uint64_t physical_addr, void* buffer, std::size_t size)
    {
        if (!physical_addr || !buffer || !size)
            return nt_status_t::invalid_parameter;

		mm_copy_address_t src{};
		src.m_physical_address.m_quad_part = physical_addr;

		std::size_t bytes_copied = 0;
		auto status = nt::mm_copy_memory(buffer, src, size, copy_physical, &bytes_copied);

		if (status != nt_status_t::success || bytes_copied != size)
		{
			nt::dbg_print("[read_direct_safe] failed: status=0x%llx copied=%zu expected=%zu",
				(std::uint64_t)status, bytes_copied, size);
			return nt_status_t::unsuccessful;
		}

		return nt_status_t::success;
    }
}