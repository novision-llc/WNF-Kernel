#pragma once

namespace control::dispatch
{
	using namespace requests;

	namespace driver
	{
        static bool recieved_unload_packet = false;

		nt_status_t is_running(PKERNEL_RUNNING_REQUEST req)
		{
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

			nt::dbg_print(oxorany("[is_running] recieved packet from usermode."));
			wcscpy((wchar_t*)req->message, oxorany(L"1"));
			return nt_status_t::success;
		}

		nt_status_t unload_driver()
		{
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            recieved_unload_packet = true;
			return nt_status_t::success;
		}
	}

	namespace perhipheral
	{
		nt_status_t mouse_event(PPERIPHERAL_REQUEST Request)
		{
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

			ULONG input_data{}; KIRQL irql{};
			nt::dbg_print(oxorany("[mouse_event] data: X = %d, Y = %d, Flags = %d"), Request->x, Request->y, Request->k);

			MOUSE_INPUT_DATA mid = { 0 };
			mid.ExtraInformation = global::mouse::extrainfo;
			mid.LastX = Request->x;
			mid.LastY = Request->y;
			mid.ButtonFlags = Request->k;

			irql = nt::kf_raise_irql(DISPATCH_LEVEL);
			global::mouse::mouse_obj.service_callback(global::mouse::mouse_obj.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
			nt::ke_lower_irql(irql);

			return nt_status_t::success;
		}
	}

	namespace process
	{
		nt_status_t get_process_base_addr(PKERNEL_BASE_REQUEST req) 
		{
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

			eprocess_t* process = nt::ps_lookup_process_by_pid((std::uint32_t)req->ProcessId);
			if (!process)
			{
				nt::dbg_print(oxorany("[get_process_base_addr] process: Couldn't locate process by PID! pid=%lu"), req->ProcessId);
				return nt_status_t::unsuccessful;
			}
            //nt::dbg_print(oxorany("[get_process_base_addr] Found peb: 0x%llx"), ((peb_t*)process->m_peb));
            //nt::dbg_print(oxorany("[get_process_base_addr] peb base addr: 0x%llx"), ((peb_t*)process->m_peb)->m_image_base_address);

			uint64_t base_addr = reinterpret_cast<uint64_t>(nt::ps_get_process_section_base_address(process));
			if (!base_addr)
			{
				nt::dbg_print(oxorany("[get_process_base_addr] base_addr: Couldn't locate process base address! base_addr=0x%llx"), base_addr);
				return nt_status_t::unsuccessful;
			}

            nt::dbg_print(oxorany("[get_process_base_addr] Found Base address: 0x%llx"), base_addr);
            req->ProcessBase = base_addr;

			if (!g_paging.scan_pages(base_addr, 0x5A4D))
			{
				nt::dbg_print(oxorany("[get_process_base_addr] scan_pages: Couldn't scan pages!"));
                return nt_status_t::unsuccessful;
			}

			uint64_t dtb = g_paging.get_dtb();
			if (!dtb)
			{
				nt::dbg_print(oxorany("[get_process_base_addr] dtb: Couldn't locate process dtb! dtb=0x%llx"), dtb);
				return nt_status_t::unsuccessful;
			}

			nt::dbg_print(oxorany("[get_process_base_addr] Found DTB: %llu"), dtb);  // Print the size of the buffer
			req->DTB = dtb;
			
			nt::ob_dereference_object(process);
			return nt_status_t::success;
		}
	}
	
	namespace memory
	{
        nt_status_t read_virtual(PKERNEL_COPY_REQUEST req)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!req->address || !req->buffer || !req->size)
                return nt_status_t::invalid_parameter;

            std::size_t remaining = req->size;
            uintptr_t   va = req->address;
            std::uint8_t* out = static_cast<std::uint8_t*>(req->buffer);

            while (remaining)
            {
                uintptr_t pa = va;
                if (req->m_do_translate)
                {
                    pa = g_paging.translate(va);
                    if (!pa)
                    {
                        nt::dbg_print("[read_virtual] translation failed! va=0x%llx", va);
                        return nt_status_t::unsuccessful;
                    }
                }

                std::size_t page_offset = pa & page_4kb_mask;
                std::size_t chunk = min(remaining, page_4kb_size - page_offset);

                auto status = mm::phys::read_direct_safe(pa, out, chunk);
                if (status != nt_status_t::success)
                {
                    nt::dbg_print("[read_virtual] read failed! va=0x%llx pa=0x%llx", va, pa);
                    return status;
                }

                va += chunk;
                out += chunk;
                remaining -= chunk;
            }

            return nt_status_t::success;
        }

		nt_status_t write_virtual(PKERNEL_COPY_REQUEST req)
		{
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!req->address || !req->buffer || !req->size)
                return nt_status_t::invalid_parameter;

			size_t total_size = req->size;
			uint64_t physical_address = g_paging.translate(req->address);
			if (!physical_address)
			{
				nt::dbg_print(oxorany("[write_virtual] physical_address: translation failed"));
				return nt_status_t::unsuccessful;
			}

			uint64_t final_size = min(PAGE_SIZE - (physical_address & 0xFFF), total_size);
			size_t submitted_bytes = 0;
			mm::phys::write_direct(physical_address, req->buffer, final_size, &submitted_bytes);
			return nt_status_t::success;
		}
	}

    namespace fortnite
    {
        nt_status_t determine_entity(PKERNEL_DETERMINE_ENTITY input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer)
                return nt_status_t::unsuccessful;

            KERNEL_DETERMINE_ENTITY packet_in{};
            nt::rtl_copy_memory(&packet_in, input_data, sizeof(KERNEL_DETERMINE_ENTITY));

            determine_entity_in in{};
            in = packet_in.m_offsets;

            nt_status_t st = nt_status_t::success;
            std::uint8_t shift = 0;

            determine_entity_out local{};
            st = batching::determine_entity(in, local);
            if (st != nt_status_t::success)
                return st;

            __try {
                nt::rtl_copy_memory(input_data->m_buffer, &local, sizeof(local));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t determine_entity_bulk(PKERNEL_DETERMINE_ENTITY_BULK input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer || !input_data->m_player_array)
                return nt_status_t::unsuccessful;

            KERNEL_DETERMINE_ENTITY_BULK packet_in{};
            nt::rtl_copy_memory(&packet_in, input_data, sizeof(packet_in));

            constexpr size_t MAX_ENTITIES = 128;
            size_t count = min(packet_in.MaxCount, MAX_ENTITIES);

            auto* out = reinterpret_cast<determine_entity_out*>(packet_in.m_buffer);

            determine_entity_in& in = packet_in.m_offsets;
            determine_entity_out local{};

            uintptr_t base = reinterpret_cast<uintptr_t>(packet_in.m_player_array);
            size_t write_idx = 0;

            __try
            {
                for (size_t idx = 0; idx < count; idx++)
                {
                    nt::rtl_zero_memory(&local, sizeof(local));

                    in.PlayerState = base + (idx * global::uintptr_size);

                    nt_status_t st = batching::determine_entity(in, local);
                    if (st != nt_status_t::success)
                        continue;

                    out[write_idx++] = local;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            __try
            {
                input_data->Count = write_idx;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t determine_bones_bulk(PKERNEL_DETERMINE_BONES_BULK input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data ||
                !input_data->m_player_array ||
                !input_data->m_out ||
                !input_data->m_transform_buffer)
            {
                return nt_status_t::unsuccessful;
            }

            KERNEL_DETERMINE_BONES_BULK packet{};
            nt::rtl_copy_memory(&packet, input_data, sizeof(packet));

            constexpr size_t MAX_ENTITIES = 128;

            if (packet.MaxCount > MAX_ENTITIES ||
                packet.bone_indexes.index_count > 20)
            {
                return nt_status_t::invalid_parameter;
            }

            size_t total_transforms =
                packet.MaxCount * packet.bone_indexes.index_count;

            __try
            {
                nt::probe_for_write(packet.m_out,
                    packet.MaxCount * sizeof(determine_bones_out),
                    alignof(determine_bones_out));

                nt::probe_for_write(packet.m_transform_buffer,
                    total_transforms * sizeof(primitives::ftransform),
                    alignof(primitives::ftransform));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            size_t global_index = 0;
            size_t valid_count = 0;
            uintptr_t base = reinterpret_cast<uintptr_t>(packet.m_player_array);

            __try
            {
                for (size_t idx = 0; idx < packet.MaxCount; ++idx)
                {
                    determine_bones_out local{};
                    local.m_transform_index = static_cast<uint32_t>(global_index);

                    packet.m_offsets.m_player_state =
                        base + (idx * global::uintptr_size);

                    nt_status_t st = batching::determine_bones(
                        packet.bone_indexes,
                        packet.m_offsets,
                        (primitives::ftransform*)packet.m_transform_buffer,
                        global_index,
                        local
                    );

                    if (st != nt_status_t::success)
                        continue;

                    ((determine_bones_out*)packet.m_out)[valid_count++] = local;
                    global_index += local.m_count;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            __try
            {
                input_data->Count = valid_count;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t determine_actor(PKERNEL_DETERMINE_ACTOR input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer)
                return nt_status_t::unsuccessful;

            nt_status_t st = nt_status_t::success;

            KERNEL_DETERMINE_ACTOR in{};
            __try {
                nt::rtl_copy_memory(&in, input_data, sizeof(KERNEL_DETERMINE_ACTOR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nt_status_t::access_violation;
            }

            actor_determine_out local{};
            auto* out = &local;

            std::uint64_t AActor{};
            st = mm::phys::km_safe_read(in.AActor, &AActor, sizeof(AActor));
            if (st != nt_status_t::success || !AActor)
                return nt_status_t::unsuccessful;

            out->Actor = AActor;

            std::uint64_t rootComp{};
            st = mm::phys::km_safe_read(AActor + in.RootComponent, &rootComp, sizeof(rootComp));
            if (st != nt_status_t::success || !rootComp)
                return nt_status_t::unsuccessful;

            primitives::fvector location{};
            st = mm::phys::km_safe_read(rootComp + in.RelativeLocation, &location, sizeof(location));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->location = location;

            float simulating = 0.0f;
            st = mm::phys::km_safe_read(AActor + in.SimulatingTooLongLength, &simulating, sizeof(simulating));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->SimulatingTooLongLength = simulating;

            std::uint8_t spawnSrc = 0;
            st = mm::phys::km_safe_read(AActor + in.PickupSpawnSource, &spawnSrc, sizeof(spawnSrc));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->PickupSpawnSource = spawnSrc;

            std::uint64_t SearchPtr = 0;
            st = mm::phys::km_safe_read(AActor + in.SearchPtr, &SearchPtr, sizeof(SearchPtr));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->SearchPtr = SearchPtr;

            std::uint64_t pickup_entry = 0;
            st = mm::phys::km_safe_read(AActor + in.PickupEntry, &pickup_entry, sizeof(pickup_entry));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->PickupEntry = pickup_entry;
            /*
            primitives::tarray_t<primitives::finstanced_struct> data_array{};
            st = mm::phys::km_safe_read(
                pickup_entry + in.m_data_array,
                &data_array,
                sizeof(data_array)
            );

            if (data_array.is_valid())
            {
                for (auto i = 0; i < data_array.m_count; i++)
                {
                    uintptr_t data_ptr = data_array.get_addr(i);
                    if (!data_ptr)
                        break;

                    primitives::finstanced_struct data{};
                    auto st = mm::phys::km_safe_read(
                        data_ptr,
                        &data,
                        sizeof(primitives::finstanced_struct)
                    );
                    if (st != nt_status_t::success)
                        continue;

                    if (data.script_struct == in.ItemRarity)
                    {
						uint8_t rarity = 0;
                        auto st = mm::phys::km_safe_read(
                            data.struct_memory,
                            &rarity,
                            sizeof(std::uint8_t)
                        );

                        out->ItemRarity = rarity;
                    }
                }
            }
            */
            std::uint64_t ItemNamePtr = 0;
            st = mm::phys::km_safe_read(pickup_entry + in.ItemNamePtr, &ItemNamePtr, sizeof(ItemNamePtr));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->ItemNamePtr = ItemNamePtr;

            __try {
                nt::rtl_copy_memory(input_data->m_buffer, out, sizeof(actor_determine_out));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t locate_actor(PKERNEL_LOCATE_ACTOR input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer)
                return nt_status_t::unsuccessful;

            nt_status_t st = nt_status_t::success;

            KERNEL_LOCATE_ACTOR in{};
            __try {
                nt::rtl_copy_memory(&in, input_data, sizeof(in));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nt_status_t::access_violation;
            }

            actor_locate_out local{};
            auto* out = &local;

            uintptr_t Actor = in.Actor;

            std::uint64_t rootComp{};
            st = mm::phys::km_safe_read(Actor + in.RootComponent, &rootComp, sizeof(uintptr_t));
            if (st != nt_status_t::success || !rootComp)
                return nt_status_t::unsuccessful;

            primitives::fvector location{};
            st = mm::phys::km_safe_read(rootComp + in.RelativeLocation, &location, sizeof(location));
            if (st != nt_status_t::success)
                return nt_status_t::unsuccessful;

            out->location = location;

            __try {
                nt::rtl_copy_memory(input_data->m_buffer, out, sizeof(actor_locate_out));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t determine_actor_bulk(PKERNEL_DETERMINE_ACTOR_BULK input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer || !input_data->m_actor_array)
                return nt_status_t::unsuccessful;

            KERNEL_DETERMINE_ACTOR_BULK packet{};
            nt::rtl_copy_memory(&packet, input_data, sizeof(packet));

            constexpr size_t MAX_ACTORS = 10000;
            size_t count = min(packet.MaxCount, MAX_ACTORS);

            auto* out = reinterpret_cast<actor_determine_out*>(packet.m_buffer);
            uintptr_t base = reinterpret_cast<uintptr_t>(packet.m_actor_array);

            size_t write_idx = 0;

            __try
            {
                for (size_t idx = 0; idx < count; ++idx)
                {
                    uintptr_t actor{};
                    nt::rtl_copy_memory(
                        &actor,
                        reinterpret_cast<void*>(base + idx * sizeof(uintptr_t)),
                        sizeof(actor));

                    actor_determine_out local{};

                    nt_status_t st =
                        batching::determine_actor(packet, actor, local);

                    if (st != nt_status_t::success)
                        continue;

                    out[write_idx++] = local;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            __try
            {
                input_data->Count = write_idx;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t locate_actor_bulk(PKERNEL_LOCATE_ACTOR_BULK input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_buffer || !input_data->m_actor_array)
                return nt_status_t::unsuccessful;

            KERNEL_LOCATE_ACTOR_BULK packet{};
            nt::rtl_copy_memory(&packet, input_data, sizeof(packet));

            constexpr size_t MAX_ACTORS = 128;
            size_t count = min(packet.MaxCount, MAX_ACTORS);

            auto* out = reinterpret_cast<actor_locate_out*>(packet.m_buffer);
            uintptr_t base = reinterpret_cast<uintptr_t>(packet.m_actor_array);

            size_t write_idx = 0;

            __try
            {
                for (size_t idx = 0; idx < count; ++idx)
                {
                    uintptr_t actor{};
                    nt::rtl_copy_memory(
                        &actor,
                        reinterpret_cast<void*>(base + idx * sizeof(uintptr_t)),
                        sizeof(actor));

                    actor_locate_out local{};

                    nt_status_t st = batching::locate_actor(
                        actor,
                        packet.RootComponent,
                        packet.RelativeLocation,
                        local);

                    if (st != nt_status_t::success)
                        continue;

                    out[write_idx++] = local;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            __try
            {
                input_data->Count = write_idx;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return nt_status_t::access_violation;
            }

            return nt_status_t::success;
        }

        nt_status_t read_array_indexes(PKERNEL_READ_ARRAY_INDEXES input_data)
        {
            if (nt::ke_get_current_irql() != PASSIVE_LEVEL)
                return nt_status_t::invalid_device_state;

            if (!input_data || !input_data->m_address || !input_data->index_count)
                return nt_status_t::unsuccessful;

            nt_status_t status = nt_status_t::unsuccessful;

            if (input_data->index_count > 128 || input_data->index_count > SIZE_MAX / input_data->m_size)
                return nt_status_t::unsuccessful;

            for (std::uint32_t i = 0; i < input_data->index_count; i++)
            {
                uintptr_t virt = input_data->m_address + input_data->indexes[i] * input_data->m_size;

                uintptr_t phys = g_paging.translate(virt);
                if (!phys)
                    continue;

                void* dst = (std::uint8_t*)input_data->m_buffer + i * input_data->m_size;

                status = mm::phys::read_direct_safe(
                    phys,
                    dst,
                    input_data->m_size
                );
            }

            return status;
        }
    }
}