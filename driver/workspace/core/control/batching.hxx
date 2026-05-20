#pragma once

namespace control::batching
{
	using namespace requests;
    using namespace global;

    nt_status_t determine_entity(determine_entity_in in, determine_entity_out& out)
    {
        nt_status_t st = nt_status_t::success;

        // Player
        st = mm::phys::km_safe_read(in.PlayerState, &out.PlayerState, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // Pawn
        st = mm::phys::km_safe_read(out.PlayerState + in.Pawn, &out.Pawn, uintptr_size);
        if (st != nt_status_t::success || !out.Pawn)
            return nt_status_t::unsuccessful;

        // Mesh
        st = mm::phys::km_safe_read(out.Pawn + in.Mesh, &out.Mesh, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // RootComponent
        uintptr_t rootComp{};
        st = mm::phys::km_safe_read(out.Pawn + in.RootComponent, &rootComp, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // LastRenderTime
        st = mm::phys::km_safe_read(out.Mesh + in.LastRenderTime, &out.LastRenderTime, sizeof(float));
        if (st != nt_status_t::success)
            return st;

        // BoneArray
        st = mm::phys::km_safe_read(out.Mesh + in.BoneArray, &out.BoneArray, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // BoneArray2
        if (!out.BoneArray)
        {
            st = mm::phys::km_safe_read(out.Mesh + in.BoneArray + 0x10, &out.BoneArray, uintptr_size);
            if (st != nt_status_t::success)
                return st;
        }

        // HabenaroComponent
        std::uintptr_t habenaroComp;
        st = mm::phys::km_safe_read(out.PlayerState + in.HabenaroComponent, &habenaroComp, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // Rank
        if (habenaroComp)
        {
            st = mm::phys::km_safe_read(habenaroComp + in.RankedProgress, &out.Rank, uint32_size);
            if (st != nt_status_t::success)
                return st;
        }

        // TeamIndex
        st = mm::phys::km_safe_read(out.PlayerState + in.TeamIndex, &out.TeamIndex, uint32_size);
        if (st != nt_status_t::success)
            return st;

        // Velocity
        if (rootComp)
        {
            st = mm::phys::km_safe_read(rootComp + in.Velocity, &out.Velocity, fvector_size);
            if (st != nt_status_t::success)
                return st;
        }

        // Bounds
        st = mm::phys::km_safe_read(out.Mesh + in.EntityBounds, &out.EntityBounds, fboxbounds_size);
        if (st != nt_status_t::success)
            return st;

        // Flags
        st = mm::phys::km_safe_read(out.Pawn + in.isDying, &out.isDying, sizeof(char));
        if (st != nt_status_t::success)
            return st;

        st = mm::phys::km_safe_read(out.Pawn + in.isDowned, &out.isDowned, sizeof(char));
        if (st != nt_status_t::success)
            return st;

        out.isDying = (out.isDying & (1 << 5)) != 0;
        out.isDowned = (out.isDowned & (1 << 7)) != 0;

        return nt_status_t::success;
    }

    nt_status_t determine_bones(
        const bone_idx& idxs,
        determine_bones_in in,
        primitives::ftransform* transform_buffer,
        size_t base_index,
        determine_bones_out& out
    )
    {
        nt_status_t st;

        if (idxs.index_count > 20)
        {
            return nt_status_t::invalid_parameter;
        }

        st = mm::phys::km_safe_read(in.m_player_state, &out.m_player_state, uintptr_size);
        if (st != nt_status_t::success)
        {
            return st;
        }

        uintptr_t pawn = 0;
        st = mm::phys::km_safe_read(out.m_player_state + in.m_pawn, &pawn, uintptr_size);
        if (st != nt_status_t::success || !pawn)
        {
            return nt_status_t::unsuccessful;
        }

        uintptr_t mesh = 0;
        st = mm::phys::km_safe_read(pawn + in.m_mesh, &mesh, uintptr_size);
        if (st != nt_status_t::success)
        {
            return st;
        }

        uintptr_t bone_array = 0;
        st = mm::phys::km_safe_read(mesh + in.m_bone_array, &bone_array, uintptr_size);
        if (st != nt_status_t::success)
        {
            return st;
        }

        if (!bone_array)
        {
            st = mm::phys::km_safe_read(mesh + in.m_bone_array + 0x10, &bone_array, uintptr_size);
            if (st != nt_status_t::success)
            {
                return st;
            }
        }

        st = mm::phys::km_safe_read(mesh + in.m_component_to_world, &out.m_component_to_world, ftransform_size);
        if (st != nt_status_t::success)
        {
            return st;
        }

        for (size_t i = 0; i < idxs.index_count; ++i)
        {
            primitives::ftransform* dst = &transform_buffer[base_index + i];
            uintptr_t bone_addr = bone_array + idxs.indexes[i] * ftransform_size;

            st = mm::phys::km_safe_read(bone_addr, dst, ftransform_size);
            if (st != nt_status_t::success)
                continue;
            
            out.m_count++;
        }

        return nt_status_t::success;
    }

    nt_status_t determine_actor(
        const KERNEL_DETERMINE_ACTOR_BULK& in,
        uintptr_t actor_ptr,
        actor_determine_out& out)
    {
        if (!actor_ptr)
            return nt_status_t::invalid_parameter;

        nt_status_t st;

        out.Actor = actor_ptr;

        uintptr_t rootComp{};
        st = mm::phys::km_safe_read(
            actor_ptr + in.RootComponent,
            &rootComp,
            sizeof(rootComp));
        if (st != nt_status_t::success || !rootComp)
            return nt_status_t::unsuccessful;

        st = mm::phys::km_safe_read(
            rootComp + in.RelativeLocation,
            &out.location,
            sizeof(out.location));
        if (st != nt_status_t::success)
            return st;

        st = mm::phys::km_safe_read(
            actor_ptr + in.SimulatingTooLongLength,
            &out.SimulatingTooLongLength,
            sizeof(float));
        if (st != nt_status_t::success)
            return st;

        st = mm::phys::km_safe_read(
            actor_ptr + in.PickupSpawnSource,
            &out.PickupSpawnSource,
            sizeof(std::uint8_t));
        if (st != nt_status_t::success)
            return st;

        st = mm::phys::km_safe_read(
            actor_ptr + in.SearchPtr,
            &out.SearchPtr,
            sizeof(uintptr_t));
        if (st != nt_status_t::success)
            return st;

        st = mm::phys::km_safe_read(
            actor_ptr + in.PickupEntry,
            &out.PickupEntry,
            sizeof(uintptr_t));
        if (st != nt_status_t::success || !out.PickupEntry)
            return nt_status_t::unsuccessful;

        /*
        primitives::tarray_t<primitives::finstanced_struct> data_array{};
        st = mm::phys::km_safe_read(
            out.PickupEntry + in.m_data_array,
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
                    auto st = mm::phys::km_safe_read(
                        data.struct_memory,
                        &out.ItemRarity,
                        sizeof(std::uint8_t)
					);
                }
            }
        }
        */

        st = mm::phys::km_safe_read(
            out.PickupEntry + in.ItemNamePtr,
            &out.ItemNamePtr,
            sizeof(uintptr_t));
        if (st != nt_status_t::success)
            return st;

        return nt_status_t::success;
    }


    nt_status_t locate_actor(
        uintptr_t actor_ptr,
        uintptr_t RootComponent,
        uintptr_t RelativeLocation,
        actor_locate_out& out)
    {
        if (!actor_ptr)
            return nt_status_t::invalid_parameter;

        nt_status_t st;

        uintptr_t rootComp{};
        st = mm::phys::km_safe_read(
            actor_ptr + RootComponent,
            &rootComp,
            sizeof(rootComp));
        if (st != nt_status_t::success || !rootComp)
            return nt_status_t::unsuccessful;

        st = mm::phys::km_safe_read(
            rootComp + RelativeLocation,
            &out.location,
            sizeof(out.location));
        if (st != nt_status_t::success)
            return st;

        return nt_status_t::success;
    }
}