#pragma once

#include "core/defines.hpp"

#include <cstdlib>

namespace rin {

constexpr i32 DARRAY_DEFAULT_CAPACITY = 8;

template<typename T>
struct darray {
    T* data;
    size_t len;
    size_t capacity;

    darray() = delete;
    darray(const darray&) = delete;
    darray& operator=(const darray&) = delete;
    darray(darray&& other) = delete;
    darray& operator=(darray&& other) = delete;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    darray(bool zeroed) noexcept
    {
        if (zeroed) {
            data = (T*)calloc(DARRAY_DEFAULT_CAPACITY, sizeof(T));
        } else {
            data = (T*)malloc(DARRAY_DEFAULT_CAPACITY * sizeof(T));
        }

        len = 0;
        capacity = DARRAY_DEFAULT_CAPACITY;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    darray(u64 initial_capacity, bool zeroed) noexcept
    {
        if (zeroed) {
            data = (T*)calloc(initial_capacity, sizeof(T));
        } else {
            data = (T*)malloc(initial_capacity * sizeof(T));
        }

        len = 0;
        capacity = initial_capacity;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~darray(void) noexcept
    {
        if (data != nullptr) {
            free(data);
        }
        data = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void reserve(u64 new_capacity) noexcept
    {
        if (new_capacity <= capacity)
            return;

        data = (T*)realloc(data, new_capacity * sizeof(T));
        capacity = new_capacity;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void push(const T& item) noexcept
    {
        if (len + 1 > capacity) {
            reserve(capacity * 2);
        }
        data[len] = item;
        len += 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void trim(void) noexcept
    {
        data = (T*)realloc(data, len * sizeof(T));
        capacity = len;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void clear(void) noexcept
    {
        len = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    T& operator[](size_t index)
    {
        return data[index];
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const T& operator[](size_t index) const
    {
        return data[index];
    }
};

}
