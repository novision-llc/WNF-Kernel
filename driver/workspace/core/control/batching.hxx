#pragma once

namespace control::batching
{
	using namespace requests;
    using namespace global;

    constexpr size_t MAX_ENTITIES = 128;

    nt_status_t determine_entity(determine_entity_in in, determine_entity_out& out)
    {
        nt_status_t st = nt_status_t::success;

        // Player
        st = mm::phys::km_safe_read(in.PlayerState, &out.PlayerState, uintptr_size);
        if (st != nt_status_t::success || !out.PlayerState)
            return st;

        // Pawn
        st = mm::phys::km_safe_read(out.PlayerState + in.Pawn, &out.Pawn, uintptr_size);
        if (st != nt_status_t::success || !out.Pawn)
            return st;

        // Mesh
        st = mm::phys::km_safe_read(out.Pawn + in.Mesh, &out.Mesh, uintptr_size);
        if (st != nt_status_t::success || !out.Mesh)
            return st;

        // RootComponent
        uintptr_t rootComp{};
        st = mm::phys::km_safe_read(out.Pawn + in.RootComponent, &rootComp, uintptr_size);
        if (st != nt_status_t::success || !rootComp)
            return st;

        // BoneArray
        st = mm::phys::km_safe_read(out.Mesh + in.BoneArray, &out.BoneArray, uintptr_size);
        if (st != nt_status_t::success)
            return st;

        // BoneArray2
        if (!out.BoneArray)
        {
            st = mm::phys::km_safe_read(out.Mesh + in.BoneArray + 0x10, &out.BoneArray, uintptr_size);
            if (st != nt_status_t::success || !out.BoneArray)
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

        uintptr_t player_state = 0;
        st = mm::phys::km_safe_read(in.m_player_state, &player_state, uintptr_size);
        if (st != nt_status_t::success || !player_state)
        {
            out.m_early_return = 11;
            out.m_error_code = st;
            return st;
        }

        uintptr_t pawn = 0;
        st = mm::phys::km_safe_read(player_state + in.m_pawn, &pawn, uintptr_size);
        if (st != nt_status_t::success || !pawn)
        {
            out.m_early_return = 22;
            out.m_error_code = st;
            return st;
        }

        uintptr_t mesh = 0;
        st = mm::phys::km_safe_read(pawn + in.m_mesh, &mesh, uintptr_size);
        if (st != nt_status_t::success || !mesh)
        {
            out.m_early_return = 33;
            out.m_error_code = st;
            return st;
        }

		st = mm::phys::km_safe_read(mesh + in.m_component_to_world, &out.m_component_to_world, ftransform_size);
        if (st != nt_status_t::success)
        {
            out.m_early_return = 44;
            out.m_error_code = st;
            return st;
        }

        st = mm::phys::km_safe_read(mesh + in.m_bone_array, &out.m_bone_array, uintptr_size);
        if (st != nt_status_t::success)
        {
            out.m_early_return = 55;
            out.m_error_code = st;
            return st;
        }

        if (!out.m_bone_array)
        {
            st = mm::phys::km_safe_read(mesh + in.m_bone_array + 0x10, &out.m_bone_array, uintptr_size);
            if (st != nt_status_t::success || !out.m_bone_array)
            {
                out.m_early_return = 66;
                out.m_error_code = st;
                return st;
            }
        }

        if (idxs.index_count > 20)
        {
            out.m_early_return = 77;
            out.m_error_code = nt_status_t::invalid_parameter;
            return nt_status_t::invalid_parameter;
        }

        for (size_t i = 0; i < idxs.index_count; ++i)
        {
            primitives::ftransform* dst = &transform_buffer[base_index + i];
            uintptr_t bone_addr = out.m_bone_array + idxs.indexes[i] * ftransform_size;

            st = mm::phys::km_safe_read(bone_addr, dst, ftransform_size);
            if (st != nt_status_t::success)
            {
                out.m_early_return = 88;
                out.m_error_code = st;
                continue;
            }
            
            out.m_count++;
        }

        return nt_status_t::success;
    }
}