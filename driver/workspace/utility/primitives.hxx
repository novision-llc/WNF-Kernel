#pragma once

namespace primitives
{
	struct fquat
	{
		double x, y, z, w;
	};

	struct fvector
	{
		double x, y, z;
	};

	struct frotator
	{
		float x, y, z;
	};

	struct ftransform
	{
		fquat m_rotation{};
		fvector m_translation{};
		uint8_t padd1[8]{};
		fvector m_scale{};
		uint8_t padd2[8]{};
	};

#pragma pack(push, 1)
	struct fboxbounds
	{
		fvector orgin;
		fvector extent;
		double radius;
	};
#pragma pack(pop)

	static_assert(sizeof(fboxbounds) == 0x38);

	template<class T>
	struct tarray_t
	{
		T* m_data;
		int32_t m_count;
		int32_t m_max;

		[[nodiscard]]
		uintptr_t get_addr(std::uintptr_t idx) const noexcept
		{
			if (m_count == 0 || idx > m_count)
				return 0;

			return reinterpret_cast<std::uintptr_t>(m_data) + (idx * sizeof(T));
		}

		bool is_valid() const noexcept
		{
			return m_data && m_count > 0 && m_count <= m_max;
		}
	};

	struct finstanced_struct
	{
		uintptr_t script_struct;
		uintptr_t struct_memory;
	};
}