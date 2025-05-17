#pragma once

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

// Divisions of a nanosecond.
constexpr u64 ns_per_us = 1000;
constexpr u64 ns_per_ms = 1000 * ns_per_us;
constexpr u64 ns_per_s = 1000 * ns_per_ms;
constexpr u64 ns_per_min = 60 * ns_per_s;
constexpr u64 ns_per_hour = 60 * ns_per_min;
constexpr u64 ns_per_day = 24 * ns_per_hour;
constexpr u64 ns_per_week = 7 * ns_per_day;

// Divisions of a microsecond.
constexpr u64 us_per_ms = 1000;
constexpr u64 us_per_s = 1000 * us_per_ms;
constexpr u64 us_per_min = 60 * us_per_s;
constexpr u64 us_per_hour = 60 * us_per_min;
constexpr u64 us_per_day = 24 * us_per_hour;
constexpr u64 us_per_week = 7 * us_per_day;

// Divisions of a millisecond.
constexpr u64 ms_per_s = 1000;
constexpr u64 ms_per_min = 60 * ms_per_s;
constexpr u64 ms_per_hour = 60 * ms_per_min;
constexpr u64 ms_per_day = 24 * ms_per_hour;
constexpr u64 ms_per_week = 7 * ms_per_day;

// Divisions of a second.
constexpr u64 s_per_min = 60;
constexpr u64 s_per_hour = s_per_min * 60;
constexpr u64 s_per_day = s_per_hour * 24;
constexpr u64 s_per_week = s_per_day * 7;
