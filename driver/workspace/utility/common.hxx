#pragma once

constexpr auto page_4kb_size = 0x1000ull;
constexpr auto page_2mb_size = 0x200000ull;
constexpr auto page_1gb_size = 0x40000000ull;

constexpr auto page_shift = 12ull;
constexpr auto page_2mb_shift = 21ull;
constexpr auto page_1gb_shift = 30ull;

constexpr auto page_4kb_mask = 0xFFFull;
constexpr auto page_2mb_mask = 0x1FFFFFull;
constexpr auto page_1gb_mask = 0x3FFFFFFFull;

enum class page_table_level {
	pml4,
	pdpt,
	pd,
	pt
};

namespace global
{
    constexpr size_t uintptr_size = sizeof(uintptr_t);
    constexpr size_t uint32_size = sizeof(uint32_t);
    
	constexpr size_t fquat_size = sizeof(primitives::fquat);
    constexpr size_t fvector_size = sizeof(primitives::fvector);
    constexpr size_t frotator_size = sizeof(primitives::frotator);
	constexpr size_t ftransform_size = sizeof(primitives::ftransform);
    constexpr size_t fboxbounds_size = sizeof(primitives::fboxbounds);

	namespace mouse
	{
		static ULONG extrainfo = 0;
		static MOUSE_OBJECT mouse_obj{};
	}

	namespace event
	{
		static uint64_t magic = 0xAE415;
		static uint64_t callback_statename = 0x0d83063ea3be0875;
		static PVOID wnf_subscription = 0;
		static kevent_t* sync_event = 0;
		static handle_t event_handle = 0;
	}
}

#define copy_physical 1
#define copy_virtual 2

#define E_FAILED(x)  (x) ? (nt_status_t)0 : nt_status_t::unsuccessful
#define E_SUCCESS(x) (x) ? (nt_status_t)1 : nt_status_t::success

namespace std
{

    template <typename type, size_t max_size = 1000>
    class c_vector {
    private:
        type m_data[max_size]{};
        size_t m_size{};
        kspin_lock_t m_lock{};

    public:
        c_vector() : m_size(0) {
            nt::ke_initialize_spin_lock(&m_lock);
        }

        c_vector(const c_vector&) = delete;
        c_vector& operator=(const c_vector&) = delete;

        bool push_back_batch(const type* values, size_t count) {
            if (!values || count == 0)
                return false;

            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            if (m_size + count > max_size) {
                nt::ke_release_spin_lock(&m_lock, old_irql);
                return false;
            }

            nt::rtl_copy_memory(&m_data[m_size], values, count * sizeof(type));
            m_size += count;
            nt::ke_release_spin_lock(&m_lock, old_irql);
            return true;
        }

        bool push_back(const type& value) {
            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            if (m_size >= max_size) {
                nt::ke_release_spin_lock(&m_lock, old_irql);
                return false;
            }

            m_data[m_size++] = value;
            nt::ke_release_spin_lock(&m_lock, old_irql);
            return true;
        }

        bool pop_back() {
            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            if (m_size == 0) {
                nt::ke_release_spin_lock(&m_lock, old_irql);
                return false;
            }

            --m_size;

            nt::ke_release_spin_lock(&m_lock, old_irql);
            return true;
        }

        void clear() {
            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            nt::rtl_zero_memory(m_data, sizeof(type) * m_size);
            m_size = 0;

            nt::ke_release_spin_lock(&m_lock, old_irql);
        }

        bool erase(size_t index) {
            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            if (index >= m_size) {
                nt::ke_release_spin_lock(&m_lock, old_irql);
                return false;
            }

            if (index < m_size - 1) {
                nt::rtl_move_memory(
                    &m_data[index],
                    &m_data[index + 1],
                    (m_size - index - 1) * sizeof(type)
                );
            }

            --m_size;

            nt::ke_release_spin_lock(&m_lock, old_irql);
            return true;
        }

        bool at(size_t index, type& out_value) {
            kirql_t old_irql;
            nt::ke_acquire_spin_lock(&m_lock, &old_irql);

            if (index >= m_size) {
                nt::ke_release_spin_lock(&m_lock, old_irql);
                return false;
            }

            out_value = m_data[index];

            nt::ke_release_spin_lock(&m_lock, old_irql);
            return true;
        }

        type operator[](size_t index) const {
            if (index >= m_size)
                return type{};
            return m_data[index];
        }

        type& operator[](size_t index) {
            return m_data[index];
        }

        type* data() { return m_data; }
        const type* data() const { return m_data; }

        const type* begin() const { return m_data; }
        const type* end() const { return m_data + m_size; }

        bool reserve(size_t new_size) {
            return new_size <= max_size;
        }

        size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        bool full() const { return m_size >= max_size; }
        size_t capacity() const { return max_size; }
    };
}