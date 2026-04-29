#pragma once

namespace device::irp
{
	ULONG pendingkey = 0;

	nt_status_t dispatch_attach(driver_object_t* driver_obj)
	{
		unicode_string_t mouclass_name = RTL_CONSTANT_STRING(L"\\Driver\\Mouclass");
		driver_object_t* target_driver_obj{};
		device_object_t* curr_device_obj{};
		device_object_t* device_obj = NULL;

		nt::dbg_print(oxorany("Before ObReferenceObjectByName"));
		nt_status_t status = nt::ob_reference_object_by_name(&mouclass_name, OBJ_CASE_INSENSITIVE, NULL, 0, mouse::IoDriverObjectType, KernelMode, NULL, (PVOID*)&target_driver_obj);
		if (status != nt_status_t::success)
		{
			nt::dbg_print(oxorany("ObReferenceObjectByName failed"));
			return status;
		}

		curr_device_obj = target_driver_obj->device_object;
		nt::ob_dereference_object(target_driver_obj);

		while (curr_device_obj)
		{
			status = nt::io_create_device(driver_obj, sizeof(DEVICE_EXTENSION), NULL, device_type_t::mouse, 0, FALSE, &device_obj);
			if (status != nt_status_t::success)
			{
				nt::dbg_print(oxorany("IoCreateDevice failed"));
				return status;
			}

			nt::rtl_zero_memory(device_obj->device_extension, sizeof(DEVICE_EXTENSION));
			nt::dbg_print(oxorany("Before IoAttachDeviceToDeviceStackSafe"));

			status = nt::io_attach_device_to_device_stack_safe(device_obj, curr_device_obj, &((PDEVICE_EXTENSION)device_obj->device_extension)->LowerKbdDevice);
			if (status != nt_status_t::success)
			{
				nt::io_delete_device(device_obj);
				nt::dbg_print(oxorany("IoAttachDeviceToDeviceStackSafe failed"));
				return status;
			}

			device_obj->flags |= DO_BUFFERED_IO;
			device_obj->flags &= ~DO_DEVICE_INITIALIZING;
			curr_device_obj = curr_device_obj->next_device;
		}

		return nt_status_t::success;
	}


	void dispatch_unload(driver_object_t* driver_obj)
	{
		if (global::event::sync_event)
		{
			nt::ob_dereference_object(global::event::sync_event); // required
			global::event::sync_event = NULL;
		}

		if (global::event::wnf_subscription)
		{
			nt::ex_unsubscribe_wnf_state_change(global::event::wnf_subscription);
		}

		large_integer_t interval = { 0 };
		device_object_t* device_obj = driver_obj->device_object;
		while (device_obj)
		{
			nt::io_detatch_device(((PDEVICE_EXTENSION)device_obj->device_extension)->LowerKbdDevice);
			device_obj = device_obj->next_device;
		}

		//todo: this might be wrong
		interval.m_quad_part = static_cast<int64_t>(-10) * 1000 * 1000; // 1 second
		while (pendingkey)
		{
			nt::ke_delay_execution_thread(KernelMode, FALSE, &interval);
		}

		device_obj = driver_obj->device_object;
		while (device_obj)
		{
			nt::io_delete_device(device_obj);
			device_obj = device_obj->next_device;
		}

		nt::dbg_print(oxorany("[driver_unload] Unloaded Driver!"));
		UNREFERENCED_PARAMETER(driver_obj);
	}

	nt_status_t dispatch_pass(device_object_t* device_obj, iop_irp_t* irp)
	{
		iop_irp_stack_profiler_t* sl = nt::io_get_current_irp_stack_location(irp);
		UNREFERENCED_PARAMETER(device_obj);

		nt::io_copy_current_irp_stack_location_to_next(irp);
		return nt::iof_call_driver(((PDEVICE_EXTENSION)device_obj->device_extension)->LowerKbdDevice, irp);
	}

	typedef struct _MOUSE_OVERRIDE
	{
		LONG LastX;
		LONG LastY;
		USHORT ButtonFlags;
		BOOLEAN Pending;
	} MOUSE_OVERRIDE;
	MOUSE_OVERRIDE g_MouseOverride = { 0 };

	nt_status_t read_complete(device_object_t* device_obj, iop_irp_t* irp, PVOID ctx)
	{
		UNREFERENCED_PARAMETER(device_obj);
		UNREFERENCED_PARAMETER(ctx);

		ULONG count = static_cast<ULONG>(irp->m_io_status.m_information / sizeof(MOUSE_INPUT_DATA));
		if (!count)
		{
			irp->m_io_status.m_status = nt_status_t::buffer_too_small;
			irp->m_io_status.m_information = 0;
			nt::iof_complete_request(irp, IO_NO_INCREMENT);
			return irp->m_io_status.m_status;
		}

		if (irp->m_io_status.m_status != nt_status_t::success)
		{
			nt::dbg_print(oxorany("[read_complete] irp completion failed: status=0x%X\n"), irp->m_io_status.m_status);
			return irp->m_io_status.m_status;
		}

		PMOUSE_INPUT_DATA mouseData = reinterpret_cast<PMOUSE_INPUT_DATA>(irp->m_associated_irp.m_system_buffer);
		for (size_t i{ 0 }; i < count; i++)
		{
			if (!global::mouse::extrainfo)
				global::mouse::extrainfo = mouseData[i].ExtraInformation;

			/*
			// Only modify once per mouse_event() call
			if (g_MouseOverride.Pending)
			{
				mouseData[i].LastX = g_MouseOverride.LastX;
				mouseData[i].LastY = g_MouseOverride.LastY;
				mouseData[i].ButtonFlags = g_MouseOverride.ButtonFlags;

				// clear flag so it won’t apply again
				g_MouseOverride.Pending = FALSE;

				nt::dbg_print("Mouse Override Applied: X=%d, Y=%d, Flags=%d",
					mouseData[i].LastX,
					mouseData[i].LastY,
					mouseData[i].ButtonFlags
				);
			}

			// Optional debug: log original / modified input
			nt::dbg_print("Mouse Input: Buttons=0x%X, LastX=%d, LastY=%d",
				mouseData[i].ButtonFlags,
				mouseData[i].LastX,
				mouseData[i].LastY
			);
			*/
		}

		if (irp->m_pending_returned)
			nt::io_mark_irp_pending(irp);

		pendingkey--; // your bookkeeping
		return nt_status_t::success;
	}

	nt_status_t dispatch_read(device_object_t* device_obj, iop_irp_t* irp)
	{
		pendingkey++;
		nt::io_copy_current_irp_stack_location_to_next(irp);
		nt::io_set_completion_routine_ex(device_obj, irp, (PIO_COMPLETION_CONTEXT)read_complete, NULL, TRUE, TRUE, TRUE);

		nt_status_t status = nt::iof_call_driver(((PDEVICE_EXTENSION)device_obj->device_extension)->LowerKbdDevice, irp);
		nt::dbg_print(oxorany("[dispatch_read] IoCallDriver status is: %d"), status);
		return status;
	}

	nt_status_t dispatch_shutdown(device_object_t* DeviceObject, iop_irp_t* Irp)
	{
		Irp->m_io_status.m_status = nt_status_t::success;
		Irp->m_io_status.m_information = 0;

		nt::dbg_print(oxorany("[dispatch_shutdown] Shutdown IRP received. Pausing or cleaning up driver tasks."));
		nt::ex_unsubscribe_wnf_state_change(global::event::wnf_subscription);
		nt::iof_complete_request(Irp, IO_NO_INCREMENT);

		return nt_status_t::success;
	}
}