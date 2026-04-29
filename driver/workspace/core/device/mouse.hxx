#pragma once

namespace device::mouse
{
	extern "C" object_type_t* IoDriverObjectType;
	
	nt_status_t init_mouse(PMOUSE_OBJECT mouse_obj)
	{
		unicode_string_t class_string;
		nt::rtl_init_unicode_string(&class_string, oxorany(L"\\Driver\\MouClass"));

		driver_object_t* class_driver_object = NULL;
		nt_status_t status = nt::ob_reference_object_by_name(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
		if (status != nt_status_t::success)
			return status;

		unicode_string_t hid_string;
		nt::rtl_init_unicode_string(&hid_string, oxorany(L"\\Driver\\MouHID"));

		driver_object_t* hid_driver_object = NULL;
		status = nt::ob_reference_object_by_name(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
		if (status != nt_status_t::success)
		{
			if (class_driver_object) { nt::ob_dereference_object(class_driver_object); }
			return status;
		}

		PVOID class_driver_base = NULL;
		device_object_t* hid_device_object = hid_driver_object->device_object;
		while (hid_device_object && !mouse_obj->service_callback)
		{
			device_object_t* class_device_object = class_driver_object->device_object;
			while (class_device_object && !mouse_obj->service_callback)
			{
				if (!class_device_object->next_device && !mouse_obj->mouse_device)
				{
					mouse_obj->mouse_device = class_device_object;
				}

				PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->device_extension;
				ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->device_object_extension - (ULONG_PTR)hid_device_object->device_extension) / 4;
				class_driver_base = class_driver_object->driver_start;
				for (size_t i{ 0 }; i < device_ext_size; i++)
				{
					if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object)
					{
						mouse_obj->service_callback = (MouseClassServiceCallback)(device_extension[i + 1]);
						break;
					}
				}
				class_device_object = class_device_object->next_device;
			}
			hid_device_object = hid_device_object->attached_device;
		}

		if (!mouse_obj->mouse_device)
		{
			device_object_t* target_device_object = class_driver_object->device_object;
			while (target_device_object)
			{
				if (!target_device_object->next_device)
				{
					mouse_obj->mouse_device = target_device_object;
					break;
				}
				target_device_object = target_device_object->next_device;
			}
		}

		nt::ob_dereference_object(class_driver_object);
		nt::ob_dereference_object(hid_driver_object);

		nt::dbg_print(oxorany("[init_mouse] Mouse initialized"));
		return nt_status_t::success;
	}
}
