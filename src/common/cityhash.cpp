// Copyright (c) 2011 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// CityHash, by Geoff Pike and Jyrki Alakuijala
//
// This file provides the CityHash64 function.
//
// It's probably possible to create even faster hash functions by
// writing a program that systematically explores some of the space of
// possible hash functions, by using SIMD instructions, or by
// compromising on hash quality.

#include <algorithm>
#include <cstring>
#include "common/cityhash.h"
#include "common/swap.h"

#ifdef __GNUC__
#define HAVE_BUILTIN_EXPECT 1
#endif
#ifdef COMMON_BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif

namespace Common {

static u64 UNALIGNED_LOAD64(const char* p) {
    u64 result;
    std::memcpy(&result, p, sizeof(result));
    return result;
}

static u32 UNALIGNED_LOAD32(const char* p) {
    u32 result;
    std::memcpy(&result, p, sizeof(result));
    return result;
}

#ifdef WORDS_BIGENDIAN
#define u32_in_expected_order(x) (swap32(x))
#define u64_in_expected_order(x) (swap64(x))
#else
#define u32_in_expected_order(x) (x)
#define u64_in_expected_order(x) (x)
#endif

#if !defined(LIKELY)
#if HAVE_BUILTIN_EXPECT
#define LIKELY(x) (__builtin_expect(!!(x), 1))
#else
#define LIKELY(x) (x)
#endif
#endif

static u64 Fetch64(const char* p) {
    return u64_in_expected_order(UNALIGNED_LOAD64(p));
}

static u32 Fetch32(const char* p) {
    return u32_in_expected_order(UNALIGNED_LOAD32(p));
}

// Some primes between 2^63 and 2^64 for various uses.
static const u64 k0 = 0xc3a5c85c97cb3127ULL;
static const u64 k1 = 0xb492b66fbe98f273ULL;
static const u64 k2 = 0x9ae16a3b2f90404fULL;

// Bitwise right rotate.  Normally this will compile to a single
// instruction, especially if the shift is a manifest constant.
static u64 Rotate(u64 val, int shift) {
    // Avoid shifting by 64: doing so yields an undefined result.
    return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static u64 ShiftMix(u64 val) {
    return val ^ (val >> 47);
}

static u64 HashLen16(u64 u, u64 v) {
    return Hash128to64(u128(u, v));
}

static u64 HashLen16(u64 u, u64 v, u64 mul) {
    // Murmur-inspired hashing.
    u64 a = (u ^ v) * mul;
    a ^= (a >> 47);
    u64 b = (v ^ a) * mul;
    b ^= (b >> 47);
    b *= mul;
    return b;
}

static u64 HashLen0to16(const char* s, std::size_t len) {
    if (len >= 8) {
        u64 mul = k2 + len * 2;
        u64 a = Fetch64(s) + k2;
        u64 b = Fetch64(s + len - 8);
        u64 c = Rotate(b, 37) * mul + a;
        u64 d = (Rotate(a, 25) + b) * mul;
        return HashLen16(c, d, mul);
    }
    if (len >= 4) {
        u64 mul = k2 + len * 2;
        u64 a = Fetch32(s);
        return HashLen16(len + (a << 3), Fetch32(s + len - 4), mul);
    }
    if (len > 0) {
        u8 a = s[0];
        u8 b = s[len >> 1];
        u8 c = s[len - 1];
        u32 y = static_cast<u32>(a) + (static_cast<u32>(b) << 8);
        u32 z = static_cast<u32>(len) + (static_cast<u32>(c) << 2);
        return ShiftMix(y * k2 ^ z * k0) * k2;
    }
    return k2;
}

// This probably works well for 16-byte strings as well, but it may be overkill
// in that case.
static u64 HashLen17to32(const char* s, std::size_t len) {
    u64 mul = k2 + len * 2;
    u64 a = Fetch64(s) * k1;
    u64 b = Fetch64(s + 8);
    u64 c = Fetch64(s + len - 8) * mul;
    u64 d = Fetch64(s + len - 16) * k2;
    return HashLen16(Rotate(a + b, 43) + Rotate(c, 30) + d, a + Rotate(b + k2, 18) + c, mul);
}

// Return a 16-byte hash for 48 bytes.  Quick and dirty.
// Callers do best to use "random-looking" values for a and b.
static std::pair<u64, u64> WeakHashLen32WithSeeds(u64 w, u64 x, u64 y, u64 z, u64 a, u64 b) {
    a += w;
    b = Rotate(b + a + z, 21);
    u64 c = a;
    a += x;
    a += y;
    b += Rotate(a, 44);
    return std::make_pair(a + z, b + c);
}

// Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.
static std::pair<u64, u64> WeakHashLen32WithSeeds(const char* s, u64 a, u64 b) {
    return WeakHashLen32WithSeeds(Fetch64(s), Fetch64(s + 8), Fetch64(s + 16), Fetch64(s + 24), a,
                                  b);
}

// Return an 8-byte hash for 33 to 64 bytes.
static u64 HashLen33to64(const char* s, std::size_t len) {
    u64 mul = k2 + len * 2;
    u64 a = Fetch64(s) * k2;
    u64 b = Fetch64(s + 8);
    u64 c = Fetch64(s + len - 24);
    u64 d = Fetch64(s + len - 32);
    u64 e = Fetch64(s + 16) * k2;
    u64 f = Fetch64(s + 24) * 9;
    u64 g = Fetch64(s + len - 8);
    u64 h = Fetch64(s + len - 16) * mul;
    u64 u = Rotate(a + g, 43) + (Rotate(b, 30) + c) * 9;
    u64 v = ((a + g) ^ d) + f + 1;
    u64 w = swap64((u + v) * mul) + h;
    u64 x = Rotate(e + f, 42) + c;
    u64 y = (swap64((v + w) * mul) + g) * mul;
    u64 z = e + f + c;
    a = swap64((x + z) * mul + y) + b;
    b = ShiftMix((z + a) * mul + d + h) * mul;
    return b + x;
}

u64 CityHash64(const char* s, std::size_t len) {
    if (len <= 32) {
        if (len <= 16) {
            return HashLen0to16(s, len);
        } else {
            return HashLen17to32(s, len);
        }
    } else if (len <= 64) {
        return HashLen33to64(s, len);
    }

    // For strings over 64 bytes we hash the end first, and then as we
    // loop we keep 56 bytes of state: v, w, x, y, and z.
    u64 x = Fetch64(s + len - 40);
    u64 y = Fetch64(s + len - 16) + Fetch64(s + len - 56);
    u64 z = HashLen16(Fetch64(s + len - 48) + len, Fetch64(s + len - 24));
    std::pair<u64, u64> v = WeakHashLen32WithSeeds(s + len - 64, len, z);
    std::pair<u64, u64> w = WeakHashLen32WithSeeds(s + len - 32, y + k1, x);
    x = x * k1 + Fetch64(s);

    // Decrease len to the nearest multiple of 64, and operate on 64-byte chunks.
    len = (len - 1) & ~static_cast<std::size_t>(63);
    do {
        x = Rotate(x + y + v.first + Fetch64(s + 8), 37) * k1;
        y = Rotate(y + v.second + Fetch64(s + 48), 42) * k1;
        x ^= w.second;
        y += v.first + Fetch64(s + 40);
        z = Rotate(z + w.first, 33) * k1;
        v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
        w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
        std::swap(z, x);
        s += 64;
        len -= 64;
    } while (len != 0);
    return HashLen16(HashLen16(v.first, w.first) + ShiftMix(y) * k1 + z,
                     HashLen16(v.second, w.second) + x);
}

} // namespace Common
