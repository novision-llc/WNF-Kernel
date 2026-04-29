#pragma once

namespace mm
{
	class c_paging
	{
		mmpfn_t* m_pfn_database{};
		std::uint16_t m_cached_pml4e{};
		std::uint16_t m_cached_pdpte{};
		std::uint16_t m_cached_pde{};

	public:
		std::c_vector< pml4e, 512 > m_pml4_table{};
		std::c_vector< pdpte, 512 > m_pdpt_table{};
		std::c_vector< pde, 512 > m_pd_table{};
		std::c_vector< pte, 512 > m_pt_table{};
		cr3 m_dtb{};

		void cleanup() {
			m_pml4_table.clear();
			m_pdpt_table.clear();
			m_pd_table.clear();
			m_pt_table.clear();

			m_cached_pml4e = 0;
			m_cached_pdpte = 0;
			m_cached_pde = 0;
			m_dtb.flags = 0;
		}

		bool setup()
		{
			if (!m_pml4_table.empty())
				return false;

			m_pfn_database = nt::get_mm_pfn_database();
			if (!m_pfn_database)
				return false;

			return true;
		}

		std::uint64_t translate(std::uint64_t virtual_address, std::uint32_t* out_page_size = nullptr)
		{
			if (nt::ke_get_current_irql() > PASSIVE_LEVEL)
				return 0;

			if (!m_dtb.flags || !m_pml4_table.data())
				return 0;

			// Canonical address check: bits 63:47 must all be 0 or all be 1
			const std::uint64_t sign_bits = virtual_address >> 47;
			if (sign_bits != 0 && sign_bits != 0x1FFFFull)
				return 0;

			// Only translate user-mode addresses
			if (virtual_address >= 0xFFFF800000000000ull)
				return 0;

			const virt_addr_t va{ virtual_address };

			// ── PML4 ────────────────────────────────────────────────────────────────
			if (va.pml4e_index >= 512)
				return 0;

			const pml4e& pml4_entry = m_pml4_table[va.pml4e_index];
			if (!pml4_entry.hard.present)
				return 0;

			const std::uint64_t pdpt_pa = pml4_entry.hard.pfn << page_shift;

			// ── PDPT ────────────────────────────────────────────────────────────────
			pdpte pdpt_entry{};
			if (phys::read_direct(
				pdpt_pa + va.pdpte_index * sizeof(pdpte),
				&pdpt_entry, sizeof(pdpte)) != nt_status_t::success)
				return 0;

			if (!pdpt_entry.hard.present)
				return 0;

			// 1GB page: bits 29:0 are the page offset
			if (pdpt_entry.hard.page_size)
			{
				// For 1GB pages the PFN field encodes bits 47:30,
				// so mask off the low 18 bits of pfn that are part of the offset
				const std::uint64_t base = (pdpt_entry.hard.pfn & ~0x3FFFFull) << page_shift;
				const std::uint64_t offset = virtual_address & page_1gb_mask;        // 0x3FFFFFFF
				if (out_page_size) *out_page_size = page_1gb_size;
				return base + offset;
			}

			const std::uint64_t pd_pa = pdpt_entry.hard.pfn << page_shift;

			// ── PD ──────────────────────────────────────────────────────────────────
			pde pd_entry{};
			if (phys::read_direct(
				pd_pa + va.pde_index * sizeof(pde),
				&pd_entry, sizeof(pde)) != nt_status_t::success)
				return 0;

			if (!pd_entry.hard.present)
				return 0;

			// 2MB page: bits 20:0 are the page offset
			if (pd_entry.hard.page_size)
			{
				// For 2MB pages the PFN field encodes bits 47:21,
				// so mask off the low 9 bits of pfn that overlap the offset
				const std::uint64_t base = (pd_entry.hard.pfn & ~0x1FFull) << page_shift;
				const std::uint64_t offset = virtual_address & page_2mb_mask;        // 0x1FFFFF
				if (out_page_size) *out_page_size = page_2mb_size;
				return base + offset;
			}

			const std::uint64_t pt_pa = pd_entry.hard.pfn << page_shift;

			// ── PT ──────────────────────────────────────────────────────────────────
			pte pt_entry{};
			if (phys::read_direct(
				pt_pa + va.pte_index * sizeof(pte),
				&pt_entry, sizeof(pte)) != nt_status_t::success)
				return 0;

			if (!pt_entry.hard.present)
				return 0;

			const std::uint64_t base = pt_entry.hard.pfn << page_shift;
			const std::uint64_t offset = virtual_address & page_4kb_mask;            // 0xFFF
			if (out_page_size) *out_page_size = page_4kb_size;
			return base + offset;
		}

		// Walks the page table rooted at physical_address for base_address,
		// caches the PML4 table if the full walk succeeds, and optionally
		// validates a known signature at base_address (e.g. MZ header).
		bool cache_pt(std::uint64_t physical_address, std::uint64_t base_address, std::uint16_t signature = 0)
		{
			virt_addr_t va{ base_address };

			// Read the PML4 table from physical memory
			pml4e pml4_table[512]{};
			if (phys::read_direct(physical_address, pml4_table, sizeof(pml4_table)) != nt_status_t::success)
				return false;

			auto& pml4_entry = pml4_table[va.pml4e_index];
			if (!pml4_entry.hard.present)
				return false;

			pdpte pdpt_entry{};
			if (phys::read_direct(
				(pml4_entry.hard.pfn << page_shift) + (va.pdpte_index * sizeof(pdpte)),
				&pdpt_entry, sizeof(pdpte)) != nt_status_t::success)
				return false;

			if (!pdpt_entry.hard.present)
				return false;

			// Bail out on 1GB pages — not a typical process mapping for a module base
			if (pdpt_entry.hard.page_size)
				return false;

			pde pd_entry{};
			if (phys::read_direct(
				(pdpt_entry.hard.pfn << page_shift) + (va.pde_index * sizeof(pde)),
				&pd_entry, sizeof(pde)) != nt_status_t::success)
				return false;

			if (!pd_entry.hard.present)
				return false;

			// Handle 2MB large pages: validate signature from the large page PA directly
			if (pd_entry.hard.page_size) {
				if (signature) {
					std::uint64_t large_pa = (pd_entry.hard.pfn << page_shift) + (base_address & page_2mb_mask);
					std::uint16_t sig{};
					if (phys::read_direct(large_pa, &sig, sizeof(sig)) != nt_status_t::success)
						return false;
					if (sig != signature)
						return false;
				}

				m_pml4_table.clear();
				if (!m_pml4_table.push_back_batch(pml4_table, 512))
					return false;

				m_cached_pml4e = va.pml4e_index;
				m_cached_pdpte = va.pdpte_index;
				m_cached_pde = va.pde_index;
				return true;
			}

			pte pt_entry{};
			if (phys::read_direct(
				(pd_entry.hard.pfn << page_shift) + (va.pte_index * sizeof(pte)),
				&pt_entry, sizeof(pte)) != nt_status_t::success)
				return false;

			if (!pt_entry.hard.present)
				return false;

			// Optionally validate a known signature at base_address (e.g. 0x5A4D for MZ)
			if (signature) {
				std::uint64_t final_pa = (pt_entry.hard.pfn << page_shift) + (base_address & page_4kb_mask);
				std::uint16_t sig{};
				if (phys::read_direct(final_pa, &sig, sizeof(sig)) != nt_status_t::success)
					return false;
				if (sig != signature)
					return false;
			}

			m_pml4_table.clear();
			if (!m_pml4_table.push_back_batch(pml4_table, 512))
				return false;

			m_cached_pml4e = va.pml4e_index;
			m_cached_pdpte = va.pdpte_index;
			m_cached_pde = va.pde_index;
			return true;
		}

		bool scan_pages(std::uint64_t base_address, std::uint16_t signature = 0)
		{
			if (!base_address)
				return false;

			const auto ranges = nt::mm_get_physical_memory_ranges();
			if (!ranges)
				return false;

			for (auto i = 0; ; i++) {
				auto& memory_range = ranges[i];

				// PHYSICAL_MEMORY_RUN sentinel: both fields zero = end of list
				if (!memory_range.m_base_page.m_quad_part &&
					!memory_range.m_page_count.m_quad_part)
					break;

				// FIX: m_base_page is a PFN, shift to get the byte address
				std::uint64_t current_pa = memory_range.m_base_page.m_quad_part << page_shift;
				const std::uint64_t page_count = memory_range.m_page_count.m_quad_part;

				nt::dbg_print(oxorany("[scan_pages] range %d: base_pa=0x%llx pages=0x%llx"),
					i, current_pa, page_count);

				// FIX: increment current_page by 1 (it's a counter, not an address);
				//      increment current_pa by page_size each iteration
				for (std::uint64_t current_page = 0;
					current_page < page_count;
					current_page++, current_pa += page_4kb_size)
				{
					// Build a candidate DTB from this physical page
					cr3 current_dtb{};
					current_dtb.pfn = current_pa >> page_shift;

					if (!current_dtb.flags)
						continue;

					// Bounds-check the PFN database entry
					auto pfn_entry = &m_pfn_database[current_dtb.pfn];
					if (!pfn_entry)
						continue;

					// FIX: removed the unreliable m_pte_frame self-reference heuristic.
					// It filters out valid DTBs on newer kernels. Let cache_pt be the
					// sole validation gate — a successful full page-table walk + optional
					// signature check is a much stronger signal.

					if (cache_pt(current_pa, base_address, signature))
					{
						m_dtb = current_dtb;
						nt::dbg_print(oxorany("[scan_pages] found DTB: 0x%llx at PA=0x%llx"),
							m_dtb.flags, current_pa);
						return true;
					}
				}
			}

			return false;
		}

		std::uintptr_t get_dtb() const
		{
			return m_dtb.flags;
		}
	};
}

mm::c_paging g_paging;

namespace mm::phys
{
	nt_status_t km_safe_read(
		std::uint64_t virtual_address,
		void* out,
		std::size_t size
	)
	{
		if (!virtual_address || !out || !size)
			return nt_status_t::invalid_parameter;

		std::uint8_t* dst = static_cast<std::uint8_t*>(out);
		std::size_t remaining = size;
		std::uint64_t va = virtual_address;

		while (remaining)
		{
			std::uint64_t pa = g_paging.translate(va);
			if (!pa)
				return nt_status_t::unsuccessful;

			// Clamp the read to the end of the current physical page
			std::size_t page_offset = pa & page_4kb_mask;
			std::size_t chunk = min(remaining, page_4kb_size - page_offset);

			auto status = mm::phys::read_direct(pa, dst, chunk);
			if (status != nt_status_t::success)
				return status;

			va += chunk;
			dst += chunk;
			remaining -= chunk;
		}

		return nt_status_t::success;
	}
}