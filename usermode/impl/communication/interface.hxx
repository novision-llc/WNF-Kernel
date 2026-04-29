#pragma once

unsigned long long WNF_SHEL_DESKTOP_APPLICATION_STARTED = 0x0d83063ea3be0875;
HANDLE event_handle = NULL;
enum _COMM_REQUESTS : ULONG
{
    m_is_running,
    m_unload_driver,
    m_mouse_event,
    m_get_proccess_base_addr,
    m_read_virtual,
    m_write_virtual,
    m_read_array_indexes,
    m_read_entity_data,
    m_locate_actor,
    m_determine_actor
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

typedef struct _KERNEL_READ_ENTITY_DATA {
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
    PVOID m_buffer;
} KERNEL_READ_ENTITY_DATA, * PKERNEL_READ_ENTITY_DATA;

typedef struct _KERNEL_LOCATE_ACTOR {
    uintptr_t Actor = 0;
    uintptr_t RootComponent = 0;
    uintptr_t RelativeLocation = 0;
    PVOID m_buffer;
} KERNEL_LOCATE_ACTOR, * PKERNEL_LOCATE_ACTOR;

typedef struct _KERNEL_DETERMINE_ACTOR {
    uintptr_t AActor = 0;
    uintptr_t SimulatingTooLongLength = 0;
    uintptr_t PickupSpawnSource = 0;
    uintptr_t SearchPtr = 0;
    uintptr_t PickupEntry = 0;
    uintptr_t ItemRarity = 0;
    uintptr_t ItemNamePtr = 0;
    uintptr_t RootComponent = 0;
    uintptr_t RelativeLocation = 0;
    PVOID m_buffer;
} KERNEL_DETERMINE_ACTOR, * PKERNEL_DETERMINE_ACTOR;

extern "C"
{
    long __stdcall NtUpdateWnfStateData(
        void* StateName,
        void* Buffer,
        unsigned long Length,
        void* TypeId,
        void* ExplicitScope,
        unsigned long MatchingChangeStamp,
        unsigned long CheckStamp);
}

std::mutex wnf_call_lock;
SRWLOCK wnf_srwlock = SRWLOCK_INIT;

enum class ThreadPriority {
    High = 1, // ESP
    Medium = 2,
    Low = 3
};

thread_local std::stack<ThreadPriority> priority_stack;

void PushPriority(ThreadPriority priority) {
    priority_stack.push(priority);
}

void PopPriority() {
    if (!priority_stack.empty()) {
        priority_stack.pop();
    }
}

ThreadPriority GetCurrentPriority() {
    return priority_stack.empty() ? ThreadPriority::High : priority_stack.top();
}

struct ScopedPriority {
    ScopedPriority(ThreadPriority priority) { PushPriority(priority); }
    ~ScopedPriority() { PopPriority(); }
};

struct ThrottleState {
    std::mutex mtx;
    std::condition_variable cv;
    int budget_remaining = 1000; // adjustable
} throttle_state;

class c_memory {
public:
    uintptr_t processbase;
    HANDLE g_hWnfEvent;
    void CallBuffer(uint32_t type, void* instruction) {
        //std::lock_guard<std::mutex> lock(wnf_call_lock);
        AcquireSRWLockExclusive(&wnf_srwlock);

        COMM_HEADER request{};
        request.Magic = 0xAE415;
        request.Type = type;
        request.Instruction = instruction;

        void* pointer = &request;

        //std::cout << "[*] Address of request buffer: " << pointer << std::endl;

        // Also print the instruction pointer inside the struct
        //std::cout << "[*] Address of instruction payload: " << instruction << std::endl;
        NtUpdateWnfStateData(&WNF_SHEL_DESKTOP_APPLICATION_STARTED, &pointer, sizeof(uint64_t), nullptr, nullptr, 0, FALSE);
        if (WaitForSingleObject(event_handle, 0) == WAIT_FAILED) {
            // printf("Failed to lock and wait for event2\n");
        }
        ResetEvent(event_handle);
        ReleaseSRWLockExclusive(&wnf_srwlock);
    }

    DWORD GetProcessIdByName(const std::wstring& processName) {
        DWORD pid = 0;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return pid;
    }

    bool IsRunning()
    {
        KERNEL_RUNNING_REQUEST request{};
        CallBuffer(_COMM_REQUESTS::m_is_running, &request);
        return (_wcsicmp(request.message, L"1") == 0);
    }

    void UnloadDriver()
    {
        CallBuffer(_COMM_REQUESTS::m_unload_driver, 0);
    }

    void MoveMouse(long x, long y)
    {
        PERIPHERAL_REQUEST request{};
        request.x = x;
        request.y = y;
        request.k = 0x0;
        CallBuffer(_COMM_REQUESTS::m_mouse_event, &request);
    }

    uint64_t GetProcessBaseAddress(ULONG PID) {
        KERNEL_BASE_REQUEST request{};
        ProcessId = PID;
        request.ProcessId = ProcessId;
        request.ProcessBase = 0;

        CallBuffer(_COMM_REQUESTS::m_get_proccess_base_addr, &request);
        //std::cout << "PID: " << request.ProcessId << std::endl;
        //std::cout << "DTB: " << request.DTB << std::endl;
        //std::cout << "Base: 0x" << std::hex << request.ProcessBase << std::endl;
        return request.ProcessBase;
    }

    template<typename T>
    T Read(uint64_t address, bool translate = true) {
        static constexpr ThreadPriority DEFAULT_PRIORITY = ThreadPriority::High;

        T result{};
        if (address == 0)
            return result;

        // Throttling
        ThreadPriority current_priority = GetCurrentPriority();
        if (current_priority != ThreadPriority::High) {
            std::unique_lock<std::mutex> lock(throttle_state.mtx);
            throttle_state.cv.wait(lock, [&] {
                return throttle_state.budget_remaining > 0;
                });
            --throttle_state.budget_remaining;
            lock.unlock();
        }

        // Actual memory read
        KERNEL_COPY_REQUEST request{};
        request.m_do_translate = translate;
        request.pid = ProcessId;
        request.size = sizeof(T);
        request.address = (unsigned long long)address;
        request.buffer = &result;

        CallBuffer(_COMM_REQUESTS::m_read_virtual, &request);

        return result;
    }

    bool Read(uintptr_t address, void* buffer, size_t size, bool translate = true) {
        if (!buffer || size == 0)
            return false;

        KERNEL_COPY_REQUEST request{};
        request.m_do_translate = translate;
        request.pid = ProcessId;
        request.address = address;
        request.buffer = buffer;
        request.size = size;

        CallBuffer(_COMM_REQUESTS::m_read_virtual, &request);
        return true; // assuming CallBuffer returns NTSTATUS
    }

    template <typename T>
    bool Write(uint64_t address, T buffer) {
        static constexpr ThreadPriority DEFAULT_PRIORITY = ThreadPriority::High;

        if (address == 0)
            return false;

        // Throttling
        ThreadPriority current_priority = GetCurrentPriority();
        if (current_priority != ThreadPriority::High) {
            std::unique_lock<std::mutex> lock(throttle_state.mtx);
            throttle_state.cv.wait(lock, [&] {
                return throttle_state.budget_remaining > 0;
                });
            --throttle_state.budget_remaining;
            lock.unlock();
        }

        // Actual memory read
        KERNEL_COPY_REQUEST request{};
        request.m_do_translate = true;
        request.pid = ProcessId;
        request.size = sizeof(T);
        request.address = (unsigned long long)address;
        request.buffer = &buffer;

        CallBuffer(_COMM_REQUESTS::m_write_virtual, &request);
        return true;
    }

    template<typename ret_t = std::uintptr_t, typename addr_t>
    void TestRead(addr_t address)
    {
        std::printf("starting at:\t0x%llx\n", address);

        size_t idx = 0;
        for (size_t i{ address }; i < address + 0x256; i += 0x8)
        {
            idx += 0x8;
            std::printf("idx=%d\taddr=0x%llx\n", idx, Read<uint64_t>(i));
        }

        std::printf("Read addr + 0x256\n");
    }

    void ReadArrayIndexes(
        uintptr_t base,
        const std::vector<int>& indexes,
        size_t elementSize,
        void* outBuffer
    )
    {
        if (indexes.empty() || indexes.size() > 128)
            return;

        KERNEL_READ_ARRAY_INDEXES data{};
        data.m_address = base;
        data.m_buffer = outBuffer;
        data.m_size = elementSize;
        data.index_count = indexes.size();

        for (size_t i = 0; i < indexes.size(); ++i)
            data.indexes[i] = indexes[i];

        CallBuffer(_COMM_REQUESTS::m_read_array_indexes, &data);
    }

    void DetermineEntity(
        const entity_packet_in& inBuffer,
        void* outBuffer
    )
    {
        KERNEL_READ_ENTITY_DATA data{};
        data.PlayerState = inBuffer.PlayerState;
        data.Pawn = inBuffer.Pawn;
        data.BoneArray = inBuffer.BoneArray;
        data.Mesh = inBuffer.Mesh;
        data.RootComponent = inBuffer.RootComponent;
        data.EntityBounds = inBuffer.EntityBounds;
        data.Velocity = inBuffer.Velocity;
        data.TeamIndex = inBuffer.TeamIndex;
        data.isDying = inBuffer.isDying;
        data.isDowned = inBuffer.isDowned;
        data.HabenaroComponent = inBuffer.HabenaroComponent;
        data.RankedProgress = inBuffer.RankedProgress;
        data.m_buffer = outBuffer;

        CallBuffer(_COMM_REQUESTS::m_read_entity_data, &data);
    }

    void DetermineActor(
        const actor_determine_in& inBuffer,
        void* outBuffer
    )
    {
        KERNEL_DETERMINE_ACTOR data{};
        data.AActor = inBuffer.AActor;
        data.SimulatingTooLongLength = inBuffer.SimulatingTooLongLength;
        data.PickupSpawnSource = inBuffer.PickupSpawnSource;
        data.SearchPtr = inBuffer.SearchPtr;
        data.PickupEntry = inBuffer.PickupEntry;
        data.ItemRarity = inBuffer.ItemRarity;
        data.ItemNamePtr = inBuffer.ItemNamePtr;
        data.RootComponent = inBuffer.RootComponent;
        data.RelativeLocation = inBuffer.RelativeLocation;
        data.m_buffer = outBuffer;

        CallBuffer(_COMM_REQUESTS::m_determine_actor, &data);
    }

    void LocateActor(
        const actor_packet_in& inBuffer,
        void* outBuffer
    )
    {
        KERNEL_LOCATE_ACTOR data{};
        data.Actor = inBuffer.Actor;
        data.RootComponent = inBuffer.RootComponent;
        data.RelativeLocation = inBuffer.RelativeLocation;
        data.m_buffer = outBuffer;

        CallBuffer(_COMM_REQUESTS::m_locate_actor, &data);
    }

    bool is_valid(const uint64_t addr)
    {
        return !(addr <= 0x400000 || addr == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(addr) == nullptr || addr > 0x7FFFFFFFFFFFFFFF);
    }

private:
    ULONG ProcessId;
};

c_memory Interface;