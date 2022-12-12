#pragma once

#include <array>
#include <cstddef>
template <class T, std::size_t N>
struct MemBox
{
    static_assert(N != 0, "zero allocating");
    using mem_array_type = typename std::array<T, N>;
    using indx_array_type = typename std::array<size_t, N + 1>;
    mem_array_type mem_pool;
    indx_array_type free_indxs;
    size_t &free_indxs_count = free_indxs[0];
    MemBox()
    {
        for (size_t i = 0; i <= N; i++)
        {
            free_indxs[i] = N - i;
        }
    }
    MemBox(const MemBox &o) noexcept = default;
    T *allocate(std::size_t n)
    {
        if (n != 1)
            throw std::invalid_argument("can't allocate bigger then 1 element.");
        if (free_indxs_count == 0)
            throw std::bad_alloc();
        size_t last_free_indx = free_indxs[free_indxs_count--];
        T *p = reinterpret_cast<T *>(&mem_pool[last_free_indx]);
        return p;
    }
    void deallocate(T *p, size_t n)
    {
        size_t indx = p - mem_pool.data();
        free_indxs[++free_indxs_count] = indx;
    };
};
/**
 * @brief Алокатор на базе массива фиксированной длинны
 *
 * @tparam T Тип элемента массива
 * @tparam N Размер массива
 */
template <class T, std::size_t N = 1>
class AllocatorOnArray
{
    static_assert(N != 0, "zero allocating");

public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;
    using mem_box = MemBox<T, N>;

    template <class U>
    struct rebind
    {
        using other = AllocatorOnArray<U, N>;
    };
    AllocatorOnArray()
    {
    }
    constexpr AllocatorOnArray(const AllocatorOnArray &oth) noexcept = default;
    template <class U, std::size_t M>
    AllocatorOnArray(const AllocatorOnArray<U, M> &o) noexcept {}

    constexpr std::size_t max_size() { return m_ ? m_.capacity : 0; };

    T *allocate(std::size_t n)
    {
        return m_.allocate(n);
    };

    void deallocate(T *p, size_t n)
    {
        m_.deallocate(p, n);
    };

    template <typename U, typename... Args>
    void construct(U *p, Args &&...args)
    {
        new (p) U(args...);
    }
    template <typename U>
    void destroy(U *p)
    {
        p->~U();
    }

    AllocatorOnArray<T, N> &operator=(const AllocatorOnArray<T, N> &other) = default;

private:
    const size_t capacity_ = N;
    mem_box m_;
};

template <class T, class U, std::size_t N>
bool operator==(const AllocatorOnArray<T, N> &, const AllocatorOnArray<U, N> &)
{
    return true;
}
template <class T, class U, std::size_t N>
bool operator!=(const AllocatorOnArray<T, N> &, const AllocatorOnArray<U, N> &)
{
    return false;
}
