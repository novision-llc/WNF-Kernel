#pragma once

namespace mm::gadgets
{
    // Supported gadget types
    enum class type : std::uint8_t {
        POP_RAX_RET,          // 0x58 0xC3
        POP_RCX_RET,          // 0x59 0xC3
        POP_RDX_RET,          // 0x5A 0xC3
        POP_RBX_RET,          // 0x5B 0xC3
        POP_RSP_RET,          // 0x5C 0xC3
        POP_RBP_RET,          // 0x5D 0xC3
        POP_RSI_RET,          // 0x5E 0xC3
        POP_RDI_RET,          // 0x5F 0xC3
        POP_R8_RET,           // 0x41 0x58 0xC3
        POP_R9_RET,           // 0x41 0x59 0xC3
        POP_R10_RET,          // 0x41 0x5A 0xC3
        POP_R11_RET,          // 0x41 0x5B 0xC3
        POP_R12_RET,          // 0x41 0x5C 0xC3
        POP_R13_RET,          // 0x41 0x5D 0xC3
        POP_R14_RET,          // 0x41 0x5E 0xC3
        POP_R15_RET,          // 0x41 0x5F 0xC3
        MOV_RAX_IMM64_RET,    // 0x48 0xB8 [imm64] 0xC3
        MOV_RCX_IMM64_RET,    // 0x48 0xB9 [imm64] 0xC3
        MOV_RDX_IMM64_RET,    // 0x48 0xBA [imm64] 0xC3
        MOV_RBX_IMM64_RET,    // 0x48 0xBB [imm64] 0xC3
        MOV_RSP_IMM64_RET,    // 0x48 0xBC [imm64] 0xC3
        MOV_RBP_IMM64_RET,    // 0x48 0xBD [imm64] 0xC3
        MOV_RSI_IMM64_RET,    // 0x48 0xBE [imm64] 0xC3
        MOV_RDI_IMM64_RET,    // 0x48 0xBF [imm64] 0xC3
        PUSH_IMM32_RET,       // 0x68 [imm32] 0xC3
        ADD_RAX_IMM32_RET,    // 0x48 0x05 [imm32] 0xC3
        ADD_RCX_IMM32_RET,    // 0x48 0x81 0xC1 [imm32] 0xC3
    };

    // Description of a gadget type
    struct descriptor {
        type            id{};
        const char* name{};
        const std::uint8_t* pattern{};      // byte sequence (including ret)
        size_t          pattern_len{};
        std::uint32_t        imm_offset{};    // offset from start where immediate is (0 if none)
        std::uint8_t         imm_size{};      // 4 or 8 bytes (0 if none)
    };

    // Define patterns (null-terminated for easy initialization)
    static const std::uint8_t pat_pop_rax_ret[] = { 0x58, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rcx_ret[] = { 0x59, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rdx_ret[] = { 0x5A, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rbx_ret[] = { 0x5B, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rsp_ret[] = { 0x5C, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rbp_ret[] = { 0x5D, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rsi_ret[] = { 0x5E, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_rdi_ret[] = { 0x5F, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r8_ret[] = { 0x41, 0x58, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r9_ret[] = { 0x41, 0x59, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r10_ret[] = { 0x41, 0x5A, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r11_ret[] = { 0x41, 0x5B, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r12_ret[] = { 0x41, 0x5C, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r13_ret[] = { 0x41, 0x5D, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r14_ret[] = { 0x41, 0x5E, 0xC3, 0x00 };
    static const std::uint8_t pat_pop_r15_ret[] = { 0x41, 0x5F, 0xC3, 0x00 };

    static const std::uint8_t pat_mov_rax_imm64_ret[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };
    static const std::uint8_t pat_mov_rcx_imm64_ret[] = { 0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };
    static const std::uint8_t pat_mov_rdx_imm64_ret[] = { 0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };
    // ... similar for other registers

    static const std::uint8_t pat_push_imm32_ret[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };
    static const std::uint8_t pat_add_rax_imm32_ret[] = { 0x48, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };
    static const std::uint8_t pat_add_rcx_imm32_ret[] = { 0x48, 0x81, 0xC1, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x00 };

    // Table of all supported gadget descriptors
    static const descriptor descriptors[] = {
        { type::POP_RAX_RET,       "pop rax; ret",       pat_pop_rax_ret,       2,  0, 0 },
        { type::POP_RCX_RET,       "pop rcx; ret",       pat_pop_rcx_ret,       2,  0, 0 },
        { type::POP_RDX_RET,       "pop rdx; ret",       pat_pop_rdx_ret,       2,  0, 0 },
        { type::POP_RBX_RET,       "pop rbx; ret",       pat_pop_rbx_ret,       2,  0, 0 },
        { type::POP_RSP_RET,       "pop rsp; ret",       pat_pop_rsp_ret,       2,  0, 0 },
        { type::POP_RBP_RET,       "pop rbp; ret",       pat_pop_rbp_ret,       2,  0, 0 },
        { type::POP_RSI_RET,       "pop rsi; ret",       pat_pop_rsi_ret,       2,  0, 0 },
        { type::POP_RDI_RET,       "pop rdi; ret",       pat_pop_rdi_ret,       2,  0, 0 },
        { type::POP_R8_RET,        "pop r8; ret",        pat_pop_r8_ret,        3,  0, 0 },
        { type::POP_R9_RET,        "pop r9; ret",        pat_pop_r9_ret,        3,  0, 0 },
        { type::POP_R10_RET,       "pop r10; ret",       pat_pop_r10_ret,       3,  0, 0 },
        { type::POP_R11_RET,       "pop r11; ret",       pat_pop_r11_ret,       3,  0, 0 },
        { type::POP_R12_RET,       "pop r12; ret",       pat_pop_r12_ret,       3,  0, 0 },
        { type::POP_R13_RET,       "pop r13; ret",       pat_pop_r13_ret,       3,  0, 0 },
        { type::POP_R14_RET,       "pop r14; ret",       pat_pop_r14_ret,       3,  0, 0 },
        { type::POP_R15_RET,       "pop r15; ret",       pat_pop_r15_ret,       3,  0, 0 },
        { type::MOV_RAX_IMM64_RET, "mov rax, imm64; ret", pat_mov_rax_imm64_ret, 11, 2, 8 },
        { type::MOV_RCX_IMM64_RET, "mov rcx, imm64; ret", pat_mov_rcx_imm64_ret, 11, 2, 8 },
        // ... add others similarly
        { type::PUSH_IMM32_RET,    "push imm32; ret",    pat_push_imm32_ret,    6,  1, 4 },
        { type::ADD_RAX_IMM32_RET, "add rax, imm32; ret", pat_add_rax_imm32_ret, 7,  2, 4 },
        { type::ADD_RCX_IMM32_RET, "add rcx, imm32; ret", pat_add_rcx_imm32_ret, 8,  3, 4 },
    };
    static const size_t num_descriptors = sizeof(descriptors) / sizeof(descriptors[0]);


	namespace rop
	{
		struct rop_gadget_info {
			std::uint64_t address{};
			gadgets::type type{};
			std::uint32_t imm_offset{};
			std::uint8_t imm_size{};
			bool used{};
		};

		std::c_vector<rop_gadget_info, 512> rop_cache{};

		rop_gadget_info current_gadget = {};
		std::uint16_t gadget_usage_count = 0;

		bool get_leaf_entry(std::uint64_t virtual_address, std::uint64_t& entry, int& level) {
			if (!g_paging.m_dtb.flags)
				return false;

			if (((virtual_address >> 47) & 0x1FFFF) != 0 &&
				((virtual_address >> 47) & 0x1FFFF) != 0x1FFFF)
				return false;

			virt_addr_t va{ virtual_address };

			if (va.pml4e_index >= g_paging.m_pml4_table.size())
				return false;
			auto& pml4e = g_paging.m_pml4_table[va.pml4e_index];
			if (!pml4e.hard.present)
				return false;

			std::uint64_t pdpt_phys = pml4e.hard.pfn << page_shift;
			pdpte pdpte_entry;
			if (phys::read_direct(pdpt_phys + va.pdpte_index * sizeof(pdpte),
				&pdpte_entry, sizeof(pdpte)) != nt_status_t::success)
				return false;
			if (!pdpte_entry.hard.present)
				return false;

			// Check for 1GB page
			if (pdpte_entry.hard.page_size) {
				entry = pdpte_entry.value;
				level = 1;
				return true;
			}

			std::uint64_t pd_phys = pdpte_entry.hard.pfn << page_shift;
			pde pde_entry;
			if (phys::read_direct(pd_phys + va.pde_index * sizeof(pde),
				&pde_entry, sizeof(pde)) != nt_status_t::success)
				return false;
			if (!pde_entry.hard.present)
				return false;

			// Check for 2MB page
			if (pde_entry.hard.page_size) {
				entry = pde_entry.value;
				level = 2;
				return true;
			}

			std::uint64_t pt_phys = pde_entry.hard.pfn << page_shift;
			pte pte_entry;
			if (phys::read_direct(pt_phys + va.pte_index * sizeof(pte),
				&pte_entry, sizeof(pte)) != nt_status_t::success)
				return false;
			if (!pte_entry.hard.present)
				return false;

			entry = pte_entry.value;
			level = 3;
			return true;
		}


		bool is_page_executable(std::uint64_t va) {
			auto va_desc = virt_addr_t{ va };
			std::uint64_t entry = 0;
			int level = 0; // 1=PDPTE, 2=PDE, 3=PTE
			if (!get_leaf_entry(va, entry, level))
				return false;

			// NX is bit 63 in all levels
			return (entry & (1ULL << 63)) == 0;
		}

		nt_status_t read_direct_via_translate(std::uint64_t va, void* buffer, size_t size) {
			std::uint64_t pa = g_paging.translate(va);
			if (!pa) return nt_status_t::unsuccessful;
			return phys::read_direct(pa, buffer, size);
		}

		nt_status_t write_direct_via_translate(std::uint64_t va, void* buffer, size_t size) {
			std::uint64_t pa = g_paging.translate(va);
			if (!pa) return nt_status_t::unsuccessful;
			return phys::write_direct(pa, buffer, size);
		}

		void scan_page_for_gadgets(std::uint64_t page_va) {
			std::uint8_t* buffer = (std::uint8_t*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'gadG');
			if (!buffer) return;

			if (read_direct_via_translate(page_va, buffer, 4096) != nt_status_t::success) {
				ExFreePool(buffer);
				return;
			}

			for (std::uint32_t offset = 0; offset <= 4096 - 2; ++offset) {
				for (size_t idx = 0; idx < gadgets::num_descriptors; ++idx) {
					const auto& desc = gadgets::descriptors[idx];
					if (offset + desc.pattern_len > 4096) continue;

					bool match = true;
					for (size_t p = 0; p < desc.pattern_len; ++p) {
						std::uint8_t expected = desc.pattern[p];
						if (desc.imm_size > 0 && p >= desc.imm_offset && p < desc.imm_offset + desc.imm_size) {
							continue; // skip immediate bytes
						}
						if (buffer[offset + p] != expected) {
							match = false;
							break;
						}
					}
					if (match) {
						rop_gadget_info info;
						info.address = page_va + offset;
						info.type = desc.id;
						info.imm_offset = desc.imm_offset;
						info.imm_size = desc.imm_size;
						info.used = false;
						rop_cache.push_back(info);
					}
				}
			}
			ExFreePool(buffer);
		}

		bool scan_all_gadgets() {
			if (!rop_cache.empty())
				return true; // already scanned

			pml4e* pml4 = (pml4e*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT1');
			pdpte* pdpt = (pdpte*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT2');
			pde* pd = (pde*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT3');
			pte* pt = (pte*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT4');

			if (!pml4 || !pdpt || !pd || !pt) {
				if (pml4) ExFreePool(pml4);
				if (pdpt) ExFreePool(pdpt);
				if (pd) ExFreePool(pd);
				if (pt) ExFreePool(pt);
				return false;
			}

			std::uint64_t cr3_phys = g_paging.m_dtb.flags & ~0xFFF;
			if (phys::read_direct(cr3_phys, pml4, 4096) != nt_status_t::success)
			{
				ExFreePool(pml4);
				ExFreePool(pdpt);
				ExFreePool(pd);
				ExFreePool(pt);
				return false;
			}

			for (int i = 0; i < 256; ++i) {
				if (!pml4[i].hard.present) continue;

				std::uint64_t pdpt_phys = pml4[i].hard.pfn << 12;
				if (phys::read_direct(pdpt_phys, pdpt, 4096) != nt_status_t::success)
					continue;

				for (int j = 0; j < 512; ++j) {
					if (!pdpt[j].hard.present) continue;

					std::uint64_t base_va = (std::uint64_t(i) << 39) | (std::uint64_t(j) << 30);

					if (pdpt[j].hard.page_size) {
						// Check if executable (NX=0)
						if (!(pdpt[j].value & (1ULL << 63))) {
							for (std::uint64_t offset = 0; offset < (1ULL << 30); offset += 0x1000) {
								std::uint64_t va = base_va + offset;
								scan_page_for_gadgets(va);
								if (rop_cache.full()) break; // stop if cache full
							}
						}
						continue;
					}

					std::uint64_t pd_phys = pdpt[j].hard.pfn << 12;
					if (phys::read_direct(pd_phys, pd, 4096) != nt_status_t::success)
						continue;

					for (int k = 0; k < 512; ++k) {
						if (!pd[k].hard.present) continue;

						std::uint64_t base_va2 = base_va | (std::uint64_t(k) << 21);

						if (pd[k].hard.page_size) {  // 2 MB page
							if (!(pd[k].value & (1ULL << 63))) {
								for (std::uint64_t offset = 0; offset < (2ULL << 20); offset += 0x1000) {
									std::uint64_t va = base_va2 + offset;
									scan_page_for_gadgets(va);
									if (rop_cache.full()) break;
								}
							}
							continue;
						}

						std::uint64_t pt_phys = pd[k].hard.pfn << 12;
						if (phys::read_direct(pt_phys, pt, 4096) != nt_status_t::success)
							continue;

						for (int l = 0; l < 512; ++l) {
							if (pt[l].hard.present && !(pt[l].value & (1ULL << 63))) { // executable
								std::uint64_t va = base_va2 | (std::uint64_t(l) << 12);
								scan_page_for_gadgets(va);
								if (rop_cache.full()) break;
							}
						}
					}
				}
			}

			ExFreePool(pml4);
			ExFreePool(pdpt);
			ExFreePool(pd);
			ExFreePool(pt);

			return !rop_cache.empty();
		}

		std::uint64_t find_new_rop_gadget(rop_gadget_info& current) {
			if (rop_cache.empty()) {
				if (!scan_all_gadgets())
					return 0;
			}

			for (size_t i{ 0 }; i < rop_cache.size(); ++i) {
				auto& info = rop_cache[i];
				if (!info.used) {
					info.used = true;
					current = info;
					gadget_usage_count = 0;
					nt::dbg_print("Found Gadget: VA=0x%llx type=%d size=%d", current.address, (int)current.type, (int)current.imm_size);
					return info.address;
				}
			}

			return 0;
		}

		bool patch_gadget(const rop_gadget_info& gadget, std::uint64_t value) {
			if (gadget.imm_size == 0) {
				nt::dbg_print("[patch] Gadget at 0x%llx has no immediate\n", gadget.address);
				return false;
			}

			std::uint64_t patch_addr = gadget.address + gadget.imm_offset;
			nt::dbg_print("[patch] Patching VA 0x%llx (gadget 0x%llx + %u) with 0x%llx\n",
				patch_addr, gadget.address, gadget.imm_offset, value);

			// Translate
			std::uint64_t pa = g_paging.translate(patch_addr);
			if (!pa) {
				nt::dbg_print("[patch] translate failed for VA 0x%llx\n", patch_addr);
				return false;
			}
			nt::dbg_print("[patch] Physical address: 0x%llx\n", pa);

			nt_status_t status;
			if (gadget.imm_size == 4) {
				std::uint32_t val32 = (std::uint32_t)value;
				status = phys::write_direct(pa, &val32, 4);
			}
			else if (gadget.imm_size == 8) {
				status = phys::write_direct(pa, &value, 8);
			}
			else {
				return false;
			}

			nt::dbg_print("[patch] write result: 0x%x (%s)\n", status,
				status == nt_status_t::success ? "SUCCESS" : "FAIL");
			return status == nt_status_t::success;
		}

		void callback_to_rop_gadget(std::uint64_t target_va) {
			if (target_va >= 0xFFFF800000000000ULL) return;

			while (true) {
				if (!current_gadget.address || gadget_usage_count >= 500) {
					std::uint64_t new_addr = find_new_rop_gadget(current_gadget);
					if (!new_addr) return;
				}

				nt::dbg_print("Current Callback Gadget: 0x%llx type=%d imm_size=%d\n",
					current_gadget.address, (int)current_gadget.type, (int)current_gadget.imm_size);
				if (patch_gadget(current_gadget, target_va)) {
					gadget_usage_count++;
					return;
				}

				// Patching failed – mark as bad and try another
				current_gadget.used = true;
				current_gadget.address = 0;
			}

			gadget_usage_count++;
		}

		void enumerate_present_pages() {
			// Read the whole PML4 (1 page) from the target CR3
			pml4e* pml4 = (pml4e*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT1');
			pdpte* pdpt = (pdpte*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT2');
			pde* pd = (pde*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT3');
			pte* pt = (pte*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 4096, 'pgT4');

			if (!pml4 || !pdpt || !pd || !pt) {
				if (pml4) ExFreePool(pml4);
				if (pdpt) ExFreePool(pdpt);
				if (pd) ExFreePool(pd);
				if (pt) ExFreePool(pt);
				return;
			}

			std::uint64_t cr3_phys = g_paging.m_dtb.flags & ~0xFFF;
			if (phys::read_direct(cr3_phys, pml4, 4096) != nt_status_t::success)
			{
				ExFreePool(pml4);
				ExFreePool(pdpt);
				ExFreePool(pd);
				ExFreePool(pt);
				return;
			}

			// User addresses use indices 0..255
			for (int i = 0; i < 256; ++i) {
				if (!pml4[i].hard.present) continue;

				std::uint64_t pdpt_phys = pml4[i].hard.pfn << 12;
				if (phys::read_direct(pdpt_phys, pdpt, 4096) != nt_status_t::success)
					continue;

				for (int j = 0; j < 512; ++j) {
					if (!pdpt[j].hard.present) continue;

					std::uint64_t base_va = (std::uint64_t(i) << 39) | (std::uint64_t(j) << 30);

					if (pdpt[j].hard.page_size) {  // 1 GB page
						// All 4KB sub-pages in this 1GB range are present
						for (std::uint64_t offset = 0; offset < (1 << 30); offset += 0x1000) {
							callback_to_rop_gadget(base_va + offset);
						}
						continue;
					}

					std::uint64_t pd_phys = pdpt[j].hard.pfn << 12;
					if (phys::read_direct(pd_phys, pd, 4096) != nt_status_t::success)
						continue;

					for (int k = 0; k < 512; ++k) {
						if (!pd[k].hard.present) continue;

						std::uint64_t base_va2 = base_va | (std::uint64_t(k) << 21);

						if (pd[k].hard.page_size) {  // 2 MB page
							for (std::uint64_t offset = 0; offset < (2 << 20); offset += 0x1000) {
								callback_to_rop_gadget(base_va2 + offset);
							}
							continue;
						}

						std::uint64_t pt_phys = pd[k].hard.pfn << 12;
						if (phys::read_direct(pt_phys, pt, 4096) != nt_status_t::success)
							continue;

						for (int l = 0; l < 512; ++l) {
							if (pt[l].hard.present) {
								std::uint64_t va = base_va2 | (std::uint64_t(l) << 12);
								callback_to_rop_gadget(va);
							}
						}
					}
				}
			}

			ExFreePool(pml4);
			ExFreePool(pdpt);
			ExFreePool(pd);
			ExFreePool(pt);
		}
	}
}
