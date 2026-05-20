#pragma once

namespace control::requests
{
	enum _COMM_REQUESTS : ULONG
	{
		m_is_running,
		m_unload_driver,
		m_mouse_event,
		m_get_proccess_base_addr,
		m_read_virtual,
		m_write_virtual,
		m_read_array_indexes,
		m_determine_bones_bulk,
		m_determine_entity,
		m_determine_entity_bulk,
		m_locate_actor,
		m_determine_actor,
		m_determine_actor_bulk,
		m_locate_actor_bulk
	};

	typedef struct _KERNEL_RUNNING_REQUEST {
		wchar_t message[128];
	} KERNEL_RUNNING_REQUEST, * PKERNEL_RUNNING_REQUEST;

	typedef struct _COMM_HEADER {
		ULONG Magic;
		ULONG Type;
		PVOID Instruction;
	} COMM_HEADER, * PCOMM_HEADER;

	typedef struct _PERIPHERAL_REQUEST
	{
		long x;
		long y;
		unsigned short k;
	} PERIPHERAL_REQUEST, * PPERIPHERAL_REQUEST;

	typedef struct _KERNEL_BASE_REQUEST {
		UINT32 ProcessId;
		UINT64 ProcessBase;
		UINT64 DTB;
	} KERNEL_BASE_REQUEST, * PKERNEL_BASE_REQUEST;

	typedef struct _KERNEL_COPY_REQUEST {
		UINT32 pid;
		uintptr_t address;
		PVOID buffer;
		uintptr_t size;
		bool m_do_translate;
	} KERNEL_COPY_REQUEST, * PKERNEL_COPY_REQUEST;

	typedef struct _KERNEL_READ_ARRAY_INDEXES {
		uintptr_t m_address{};
		std::uint32_t indexes[128];
		std::uint32_t index_count = 0;
		std::size_t m_size{};
		PVOID m_buffer;
	} KERNEL_READ_ARRAY_INDEXES, * PKERNEL_READ_ARRAY_INDEXES;

	struct determine_entity_in
	{
		uintptr_t PlayerState = 0;
		uintptr_t Pawn = 0;
		uintptr_t BoneArray = 0;
		uintptr_t Mesh = 0;
		uintptr_t RootComponent = 0;
		uintptr_t EntityBounds = 0;
		uintptr_t Velocity = 0;
		uintptr_t TeamIndex = 0;
		uintptr_t isDying = 0;
		uintptr_t isDowned = 0;
		uintptr_t HabenaroComponent = 0;
		uintptr_t RankedProgress = 0;
		uintptr_t LastRenderTime = 0;
	};

	typedef struct _KERNEL_DETERMINE_ENTITY {
		determine_entity_in m_offsets;
		PVOID m_buffer;
	} KERNEL_DETERMINE_ENTITY, * PKERNEL_DETERMINE_ENTITY;

	typedef struct _KERNEL_DETERMINE_ENTITY_BULK {
		uintptr_t* m_player_array;
		determine_entity_in m_offsets;
		PVOID m_buffer;
		size_t Count;
		size_t MaxCount;
	} KERNEL_DETERMINE_ENTITY_BULK, *PKERNEL_DETERMINE_ENTITY_BULK;

	struct determine_entity_out
	{
		uintptr_t PlayerState = 0;
		uintptr_t Pawn = 0;
		uintptr_t BoneArray = 0;
		uintptr_t Mesh = 0;
		primitives::fboxbounds EntityBounds{};
		primitives::fvector Velocity{};
		std::int32_t Rank = 0;
		std::int32_t TeamIndex = 0;
		float LastRenderTime = 0.0f;
		char isDying = 0;
		char isDowned = 0;
	};

	struct bone_idx
	{
		std::uint32_t indexes[20];
		std::uint32_t index_count = 0;
	};

	struct determine_bones_in
	{
		uintptr_t m_player_state = 0;
		uintptr_t m_pawn = 0;
		uintptr_t m_mesh = 0;
		uintptr_t m_component_to_world = 0;
		uintptr_t m_bone_array = 0;
	};

	struct determine_bones_out
	{
		primitives::ftransform m_component_to_world{};
		uintptr_t m_player_state = 0;
		uint32_t m_transform_index = 0; // index into transform buffer
		uint32_t m_count = 0;
	};

	typedef struct _KERNEL_DETERMINE_BONES_BULK
	{
		uintptr_t* m_player_array;

		bone_idx bone_indexes;
		determine_bones_in m_offsets;

		// OUTPUT
		PVOID m_out;                 // per-player metadata
		PVOID m_transform_buffer; // all bones (flat)

		size_t Count;     // OUT: valid players processed
		size_t MaxCount;  // IN: max players
	} KERNEL_DETERMINE_BONES_BULK, * PKERNEL_DETERMINE_BONES_BULK;

	typedef struct _KERNEL_LOCATE_ACTOR {
		uintptr_t Actor = 0;
		uintptr_t RootComponent = 0;
		uintptr_t RelativeLocation = 0;
		PVOID m_buffer;
	} KERNEL_LOCATE_ACTOR, * PKERNEL_LOCATE_ACTOR;

	struct actor_locate_out
	{
		primitives::fvector location{};
	};

	typedef struct _KERNEL_DETERMINE_ACTOR {
		uintptr_t AActor = 0;
		uintptr_t SimulatingTooLongLength = 0;
		uintptr_t PickupSpawnSource = 0;
		uintptr_t SearchPtr = 0;
		uintptr_t PickupEntry = 0;
		uintptr_t m_data_array = 0;
		uintptr_t ItemRarity = 0;
		uintptr_t ItemNamePtr = 0;
		uintptr_t RootComponent = 0;
		uintptr_t RelativeLocation = 0;
		PVOID m_buffer;
	} KERNEL_DETERMINE_ACTOR, * PKERNEL_DETERMINE_ACTOR;

	struct actor_determine_out
	{
		uintptr_t Actor = 0;
		float SimulatingTooLongLength = -1.0f;
		std::uint8_t PickupSpawnSource = -1;
		uintptr_t SearchPtr = 0;
		primitives::fvector location{};
		uintptr_t PickupEntry = 0;
		std::uint8_t ItemRarity;
		uintptr_t ItemNamePtr = 0;
	};

	typedef struct _KERNEL_DETERMINE_ACTOR_BULK
	{
		uintptr_t* m_actor_array;

		// Same offsets as single determine_actor
		uintptr_t SimulatingTooLongLength = 0;
		uintptr_t PickupSpawnSource = 0;
		uintptr_t SearchPtr = 0;
		uintptr_t PickupEntry = 0;
		uintptr_t m_data_array = 0;
		uintptr_t ItemRarity = 0;
		uintptr_t ItemNamePtr = 0;
		uintptr_t RootComponent = 0;
		uintptr_t RelativeLocation = 0;

		// Output buffer (array of actor_determine_out)
		PVOID m_buffer;

		// OUT
		size_t Count = 0;

		// IN
		size_t MaxCount = 0;
	} KERNEL_DETERMINE_ACTOR_BULK, * PKERNEL_DETERMINE_ACTOR_BULK;


	typedef struct _KERNEL_LOCATE_ACTOR_BULK
	{
		uintptr_t* m_actor_array;

		uintptr_t RootComponent = 0;
		uintptr_t RelativeLocation = 0;

		// Output buffer (array of actor_locate_out)
		PVOID m_buffer;

		// OUT
		size_t Count = 0;

		// IN
		size_t MaxCount = 0;
	} KERNEL_LOCATE_ACTOR_BULK, * PKERNEL_LOCATE_ACTOR_BULK;

}
