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
}