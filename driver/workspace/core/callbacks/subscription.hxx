#pragma once

namespace callbacks::subscription
{
	using namespace control::requests;
	using namespace control::dispatch;

	WNF_STATE_NAME split_wnf_state_name(uint64_t fullStateName)
	{
		WNF_STATE_NAME stateName{};

		// Extract the lower and upper 32 bits from the 64-bit state name
		stateName.Data[0] = static_cast<uint32_t>(fullStateName & 0xFFFFFFFF);     // Lower 32 bits
		stateName.Data[1] = static_cast<uint32_t>((fullStateName >> 32) & 0xFFFFFFFF); // Upper 32 bits
		return stateName;
	}

	nt_status_t subscribe_callback(PVOID wnfStruct, PCWNF_STATE_NAME stateName, ULONG eventMask, ULONG changeStamp, PVOID typeId, PVOID callbackContext)
	{
		//Unreference contexts, only buffer needed is queried manually via StateData
		UNREFERENCED_PARAMETER(stateName);
		UNREFERENCED_PARAMETER(eventMask);
		UNREFERENCED_PARAMETER(changeStamp);
		UNREFERENCED_PARAMETER(typeId);
		UNREFERENCED_PARAMETER(callbackContext);

		nt_status_t status = nt_status_t::success;
		ULONG bufferSize = 0x0;
		PVOID pStateData = 0;
		ULONG actualChangeStamp = 0;

		status = nt::ex_query_wnf_state_data(wnfStruct, &actualChangeStamp, NULL, &bufferSize);
		if (status != nt_status_t::buffer_too_small)
		{
			nt::dbg_print(oxorany("[SubscribeCallback] nt::ex_query_wnf_state_data: Buffer too small!"));
			return status;
		}

		pStateData = nt::ex_allocate_pool_with_tag(NonPagedPool, bufferSize, oxorany('WNFB'));
		if (!nt::mm_is_address_valid(pStateData))
		{
			nt::dbg_print(oxorany("[SubscribeCallback] pStateData: Null buffer!"));
			return nt_status_t::insufficient_resources;
		}

		status = nt::ex_query_wnf_state_data(wnfStruct, &actualChangeStamp, pStateData, &bufferSize);
		if (status != nt_status_t::success)
		{
			nt::dbg_print(oxorany("[SubscribeCallback] nt::ex_query_wnf_state_data: Couldn't query state data!"));
			return status;
		}

		PCOMM_HEADER Data = *reinterpret_cast<PCOMM_HEADER*>(pStateData);
		if (nt::mm_is_address_valid(Data) && Data->Magic == global::event::magic)
		{
			switch (Data->Type) {
			case _COMM_REQUESTS::m_is_running:
				driver::is_running(reinterpret_cast<PKERNEL_RUNNING_REQUEST>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_unload_driver:
				driver::unload_driver();
				break;
			case _COMM_REQUESTS::m_mouse_event:
				perhipheral::mouse_event(reinterpret_cast<PPERIPHERAL_REQUEST>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_get_proccess_base_addr:
				process::get_process_base_addr(reinterpret_cast<PKERNEL_BASE_REQUEST>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_read_virtual:
				memory::read_virtual(reinterpret_cast<PKERNEL_COPY_REQUEST>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_write_virtual:
				memory::write_virtual(reinterpret_cast<PKERNEL_COPY_REQUEST>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_read_array_indexes:
				fortnite::read_array_indexes(reinterpret_cast<PKERNEL_READ_ARRAY_INDEXES>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_determine_bones_bulk:
				fortnite::determine_bones_bulk(reinterpret_cast<PKERNEL_DETERMINE_BONES_BULK>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_determine_entity:
				fortnite::determine_entity(reinterpret_cast<PKERNEL_DETERMINE_ENTITY>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_determine_entity_bulk:
				fortnite::determine_entity_bulk(reinterpret_cast<PKERNEL_DETERMINE_ENTITY_BULK>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_determine_actor:
				fortnite::determine_actor(reinterpret_cast<PKERNEL_DETERMINE_ACTOR>(Data->Instruction));
				break;
			case _COMM_REQUESTS::m_locate_actor:
				fortnite::locate_actor(reinterpret_cast<PKERNEL_LOCATE_ACTOR>(Data->Instruction));
				break;
			default:
				nt::dbg_print(oxorany("[subscribe_callback] Recieved invalid request code: %ul"), Data->Type);
				break;
			}
			nt::ke_set_event(global::event::sync_event, HIGH_PRIORITY, FALSE);
		}

		if (pStateData)
		{
			nt::ex_free_pool_with_tag(pStateData, oxorany('WNFB'));
		}

		return status;
	}
}