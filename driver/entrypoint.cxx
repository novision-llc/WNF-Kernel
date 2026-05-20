#include <impl/includes.h>

struct dpc_context_t
{
	std::uint64_t m_magic;
	std::uint64_t m_image_base;
	std::uint32_t m_image_size;
	std::uint64_t m_pdb_symbols;
}; 

static bool entry_point_impl(bool manual_mapped)
{
	unicode_string_t event_name2;
	nt::rtl_init_unicode_string(&event_name2, oxorany(L"\\BaseNamedObjects\\AudioLink"));
	
	global::event::sync_event = nt::io_create_notification_event(&event_name2, &global::event::event_handle);
	nt::ke_clear_event(global::event::sync_event);
	nt::dbg_print(oxorany("[entry_point_impl] Created sync event!"));
	
	WNF_STATE_NAME stateName = callbacks::subscription::split_wnf_state_name(global::event::callback_statename);
	nt::dbg_print(oxorany("[entry_point_impl] Aquired statename!"));

	// requires device object rn
	if (!manual_mapped)
	{
		nt::dbg_print(oxorany("[entry_point_impl] Creating mouse device!"));
		device::mouse::init_mouse(&global::mouse::mouse_obj);
	}
		
	nt::ex_subscribe_wnf_state_change(&global::event::wnf_subscription, &stateName, 3, NULL, callbacks::subscription::subscribe_callback, (PVOID)1);
	nt::dbg_print(oxorany("[entry_point_impl] Subscribed callback!"));

	if (!g_paging.setup())
	{
		nt::dbg_print(oxorany("[entry_point_impl] Failed to setup paging!"));
		return false;
	}
	nt::dbg_print(oxorany("[entry_point_impl] Setup g_paging successfully!"));

	return true;
}

nt_status_t entry_point(driver_object_t* driver_obj, unicode_string_t* reg_path)
{
	nt::m_module_base = nt::get_nt_base();

	dpc_context_t* ctx = reinterpret_cast<dpc_context_t*>(reg_path);
	if (!ctx)
		return E_FAILED(true);

	bool manual_mapped = (nt::mm_is_address_valid(ctx) && ctx->m_magic == 0x1337);
	if (manual_mapped)
	{
		nt::dbg_print(oxorany("[entry_point] Called: entry_point(null, ctx); via Manual Mapping."));
	}
	else
	{
		nt::dbg_print(oxorany("[entry_point] Called: entry_point(driver_obj, reg_path); via Windows."));
	}

	if (!entry_point_impl(manual_mapped))
	{
		return E_FAILED(manual_mapped);
	}

	if (!manual_mapped && reg_path)
	{
		UNREFERENCED_PARAMETER(reg_path);
	}
	
	if (!manual_mapped && driver_obj)
	{
		for (size_t i{ 0 }; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		{
			switch (i)
			{
			case IRP_MJ_SHUTDOWN:
				driver_obj->major_function[IRP_MJ_SHUTDOWN] = device::irp::dispatch_shutdown;
				break;
			case IRP_MJ_READ:
				driver_obj->major_function[IRP_MJ_READ] = device::irp::dispatch_read;
				break;
			default:
				driver_obj->major_function[i] = device::irp::dispatch_pass;
				break;
			}
		}

		driver_obj->driver_unload = device::irp::dispatch_unload;

		nt_status_t status = device::irp::dispatch_attach(driver_obj);
		if (status != nt_status_t::success)
		{
			nt::dbg_print(oxorany("dispatch_attach failed\n"));
			return status;
		}
	}
	
	return E_SUCCESS(manual_mapped);
}