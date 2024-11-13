/* A Prelude for C++ */
#pragma once

/* ---------------- Definitions ---------------- */
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <limits.h>
#include <atomic>
#include <source_location>

#define USE_NOEXCEPT_ON_STDLIB 1

#define caller_location \
	std::source_location const& source_location = std::source_location::current()

using Source_Location = std::source_location;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using uint = unsigned int;
using byte = uint8_t;

using isize = ptrdiff_t;
using usize = size_t;

using uintptr = uintptr_t;

using f32 = float;
using f64 = double;

using cstring = char const *;

using rune = i32;
using string = struct string;

static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad float size");
static_assert(sizeof(isize) == sizeof(usize), "Mismatched (i/u)size");
static_assert(CHAR_BIT == 8, "Invalid char size");

template<typename A, typename B = A>
struct pair {
	A first;
	B second;
};

template<typename T> constexpr
T min(T a, T b){
	return a < b ? a : b;
};

template<typename T> constexpr
T max(T a, T b){
	return a > b ? a : b;
};

template<typename T> constexpr
T clamp(T lo, T x, T hi){
	return min(max(lo, x), hi);
};

#define containerof(Ptr, Type, Member) \
	((Type *)(((void *)(Ptr)) - offsetof(Type, Member)))

/* ---------------- Defer ---------------- */
namespace _defer_impl {
template<typename F>
struct deferred_call {
	F f;
	constexpr deferred_call(F f) : f(f) {}
	~deferred_call(){ f(); }
};

template<typename F>
auto make_deferred_call(F f){
	return deferred_call<F>(f);
}

#define _defer_glue_0(X, Y) X##Y
#define _defer_glue_1(X, Y) _defer_glue_0(X, Y)
#define _defer_var(X) _defer_glue_1(X, __COUNTER__)
#define defer(Code) auto _defer_var(_deferred_call_) = ::_defer_impl::make_deferred_call([&](){ Code ; });
}

/* ---------------- Atomic ---------------- */
// Just wrap the C++ builtins, but enforce explicitness for load/store/exchange/CAS/CAW
namespace atomic {
enum struct Memory_Order {
	relaxed = int(std::memory_order_relaxed),
	consume = int(std::memory_order_consume),
	acquire = int(std::memory_order_acquire),
	release = int(std::memory_order_release),
	acq_rel = int(std::memory_order_acq_rel),
	seq_cst = int(std::memory_order_seq_cst),
};

template<typename T>
static inline constexpr
bool exchange(std::atomic<T>* obj, T desired, Memory_Order order){
	return std::atomic_exchange_explicit(obj, desired, static_cast<std::memory_order>(order));
}

template<typename T>
static inline constexpr
T load(std::atomic<T>* obj, Memory_Order order){
	return std::atomic_load_explicit(obj, static_cast<std::memory_order>(order));
}

template<typename T>
static inline constexpr
void store(std::atomic<T>* obj, T value, Memory_Order order){
	std::atomic_store_explicit(obj, value, static_cast<std::memory_order>(order));
}

template<typename T>
static inline constexpr
bool compare_exchange_strong(std::atomic<T>* obj, T* expected, T desired, Memory_Order order){
	return std::atomic_compare_exchange_strong_explicit(obj, expected, desired, static_cast<std::memory_order>(order));
}

template<typename T>
static inline constexpr
bool compare_exchange_weak(std::atomic<T>* obj, T* expected, T desired, Memory_Order order){
	return std::atomic_compare_exchange_weak_explicit(obj, expected, desired, static_cast<std::memory_order>(order));
}
}

/* ---------------- Assert & Panic ---------------- */
#define MAX_PANIC_MSG_LEN 1024

#if USE_NOEXCEPT_ON_STDLIB
#define PRELUDE_NOEXCEPT noexcept
#else
#define PRELUDE_NOEXCEPT
#endif

extern "C" {
/* WARN: may break on some systems, remove/include `noexcept` as needed. */
[[noreturn]]
void abort() PRELUDE_NOEXCEPT;
int snprintf (char *, size_t, char const *, ...) PRELUDE_NOEXCEPT;
int puts (char const*);
}

[[noreturn]] static inline
void panic(cstring msg, caller_location){
	char buf[MAX_PANIC_MSG_LEN];
	int n = snprintf(buf, MAX_PANIC_MSG_LEN - 1, "%s:%d Panic: %s", source_location.file_name(), source_location.line(), msg);
	buf[n] = 0;
	puts(buf);
	abort();
}

static inline constexpr
void _assert_impl(bool predicate, cstring msg, caller_location){
	[[unlikely]] if(!predicate){
		char buf[MAX_PANIC_MSG_LEN] = {0};
		snprintf(buf, MAX_PANIC_MSG_LEN - 1, "%s:%d Assertion failed: %s\n", source_location.file_name(), source_location.line(), msg);
		puts(buf);
		abort();
	}
}

static inline constexpr
void assert(bool predicate, cstring msg, caller_location){
#ifndef DISABLE_ASSERT
	_assert_impl(predicate, msg, source_location);
#else
	(void)predicate; (void)msg; (void)caller_location;
#endif
}

static inline constexpr
void bounds_check(bool predicate, cstring msg, caller_location){
#ifndef DISABLE_BOUNDS_CHECK
	_assert_impl(predicate, msg, source_location);
#else
	(void)predicate; (void)msg; (void)caller_location;
#endif
}

#undef MAX_PANIC_MSG_LEN
#undef USE_NOEXCEPT_ON_STDLIB
#undef PRELUDE_NOEXCEPT

/* ---------------- Option type ---------------- */
template<typename T>
struct Option {
	union { T _value; };
	bool _has_value = false;

	T unwrap(caller_location) const {
		if(!_has_value){
			panic("Attempt to unwrap empty optional", source_location);
		}
		return _value;
	}

	T unwrap_unchecked() const {
		return _value;
	}

	T unwrap_or(T alt) const {
		if(!_has_value){
			return alt;
		}
		return _value;
	}

	bool ok() const {
		return _has_value;
	}

	void clear(){
		_has_value = false;
	}

	void operator=(T x){
		_value = x;
		_has_value = true;
	}

	Option() : _has_value{false} {}
	Option(T x) : _value{x}, _has_value{true} {}
};

/* ---------------- Result type ---------------- */
template<typename T, typename E>
struct Result {
private:
	union {
		T _value;
		E _error;
	};
	bool _has_value = false;

public:
	T unwrap(caller_location) const {
		if(!_has_value){
			panic("Cannot unwrap error value", source_location);
		}
		return _value;
	}

	T unwrap_or(T alt) const {
		if(!_has_value){
			return alt;
		}
		return _value;
	}

	T unwrap_unchecked() const {
		return _value;
	}

	E unwrap_err_unchecked() const {
		return _error;
	}

	bool ok() const {
		return _has_value;
	}

	void operator=(T x){
		_value = x;
		_has_value = true;
	}

	void operator=(E x){
		_error = x;
		_has_value = false;
	}

	Result() : _error{0}, _has_value{false} {}
	Result(T x) : _value{x}, _has_value{true} {}
	Result(E x) : _error{x}, _has_value{false} {}
};

/* ---------------- Memory ---------------- */
#if defined(__clang__) || defined(__GNUC__)
#define _memset_impl __builtin_memset
#define _memcpy_impl __builtin_memcpy
#define _memmove_impl __builtin_memmove
#define _memcmp_impl __builtin_memcmp
#else
extern void* memset(void *dst, int c, size_t n);
extern void* memmove(void *dst, void const * src, size_t n);
extern void* memcpy(void *dst, void const * src, size_t n);
extern int memcmp(void const * lhs, void const * rhs, size_t n);

#define _memset_impl memset
#define _memcpy_impl memcpy
#define _memmove_impl memmove
#define _memcmp_impl memcmp
#endif

namespace mem {
template<typename T>
void swap_bytes(T* data){
	byte * raw_data = (byte*)data;
	constexpr auto len = sizeof(T);
	for(isize i = 0; i < (len / 2); i += 1){
		byte temp = raw_data[i];
		data[i] = raw_data[len - (i + 1)];
		raw_data[len - (i + 1)] = temp;
	}
}

static inline
void copy(void* dest, void const* source, isize nbytes){
	bounds_check(nbytes >= 0, "Cannot copy < 0 bytes");
	_memmove_impl(dest, source, nbytes);
}

static inline
void copy_no_overlap(void* dest, void const* source, isize nbytes){
	bounds_check(nbytes >= 0, "Cannot copy < 0 bytes");
	_memcpy_impl(dest, source, nbytes);
}

static inline
void set(void* dest, byte val, isize nbytes){
	bounds_check(nbytes >= 0, "Cannot copy < 0 bytes");
	_memset_impl(dest, val, nbytes);
}

static inline
int compare(void const* lhs, void const * rhs, isize nbytes){
	bounds_check(nbytes >= 0, "Cannot compare < 0 bytes");
	return _memcmp_impl(lhs, rhs, nbytes);
}

constexpr inline isize KiB = 1024ll;
constexpr inline isize MiB = 1024ll * 1024ll;
constexpr inline isize GiB = 1024ll * 1024ll * 1024ll;

static inline constexpr
bool valid_alignment(isize align){
	return (align & (align - 1)) == 0 && (align != 0);
}

static inline
uintptr align_forward(uintptr p, uintptr a){
	assert(valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1);
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}
}

#undef _memset_impl
#undef _memcpy_impl
#undef _memmove_impl
#undef _memcmp_impl

/* ---------------- C++ Iterator Compatibility ---------------- */
// NOTE: C++ iterators are horrible. This is *Exclusively* so I can use for(auto elem : items)
// syntax. I would nuke this section if I could.
namespace cpp_iter {
template<typename T>
struct Contigous_Memory_Iterator {
	T* data;

	using reference = T&;
	using pointer = T*;
	using value_type = T;

	constexpr reference operator*() const { return *data; }
	constexpr pointer operator->() { return data; }

	constexpr void operator++(){ data += 1; }
	constexpr void operator++(int){ data += 1; }

	constexpr bool operator!=(Contigous_Memory_Iterator rhs){ return data != rhs.data; }
	constexpr Contigous_Memory_Iterator(T* ptr) : data(ptr) {}
};

template<typename T>
struct Indexed_Contigous_Memory_Iterator {
	T* data;
	isize idx;

	using reference = T&;
	using pointer = T*;
	using value_type = T;

	constexpr pair<value_type, isize> operator*() const { return {*data, idx}; }
	constexpr pair<pointer, isize> operator->() { return {data, idx}; }

	constexpr void operator++(){ data += 1; idx += 1; }
	constexpr void operator++(int){ data += 1; idx += 1; }

	constexpr bool operator!=(Indexed_Contigous_Memory_Iterator rhs){ return data != rhs.data; }
	constexpr Indexed_Contigous_Memory_Iterator(T* ptr) : data(ptr), idx{0} {}
};
}

/* ---------------- Vector support ----------------*/
template<typename T, int N>
struct vec {
  T data[N];
  constexpr T& operator[](int i){ return data[i]; }
  constexpr T const& operator[](int i) const { return data[i]; }
};
template<typename T, int N> constexpr vec<T, N> operator+(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] + b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator+(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] + s; return r; }
template<typename T, int N> constexpr vec<T, N> operator+(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s + a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator+(vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = + a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator-(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] - b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator-(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] - s; return r; }
template<typename T, int N> constexpr vec<T, N> operator-(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s - a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator-(vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = - a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator*(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] * b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator*(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] * s; return r; }
template<typename T, int N> constexpr vec<T, N> operator*(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s * a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator/(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] / b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator/(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] / s; return r; }
template<typename T, int N> constexpr vec<T, N> operator/(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s / a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator%(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] % b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator%(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] % s; return r; }
template<typename T, int N> constexpr vec<T, N> operator%(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s % a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator&(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] & b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator&(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] & s; return r; }
template<typename T, int N> constexpr vec<T, N> operator&(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s & a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator|(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] | b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator|(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] | s; return r; }
template<typename T, int N> constexpr vec<T, N> operator|(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s | a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator^(vec<T,N> a, vec<T,N> b){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] ^ b[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator^(vec<T,N> a, T s){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] ^ s; return r; }
template<typename T, int N> constexpr vec<T, N> operator^(T s, vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = s ^ a[i]; return r; }
template<typename T, int N> constexpr vec<T, N> operator~(vec<T,N> a){ vec<T, N> r{}; for(int i=0;i<N;i++) r[i] = ~ a[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator&&(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] && b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator||(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] || b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator==(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] == b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator!=(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] != b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator>=(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] >= b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator<=(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] <= b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator>(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] > b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator<(vec<T,N> a, vec<T,N> b){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = a[i] < b[i]; return r; }
template<typename T, int N> constexpr vec<bool, N> operator!(vec<T,N> a){ vec<bool, N> r{}; for(int i=0;i<N;i++) r[i] = ! a[i]; return r; }

/* ---------------- Slices ---------------- */
template<typename T>
struct slice {
private:
	T* _data = nullptr;
	isize _len = 0;

public:
	constexpr
	auto raw_data() const { return _data; }

	constexpr
	auto len() const { return _len; }

	constexpr
	T& operator[](isize idx){
		bounds_check(idx >= 0 && idx < _len, "Out of bounds access to slice");
		return _data[idx];
	}

	constexpr
	T const& operator[](isize idx) const{
		bounds_check(idx >= 0 && idx < _len, "Out of bounds access to slice");
		return _data[idx];
	}

	constexpr
	slice<T> sub(isize start, isize n){
		if(start < 0 || n < 0 || (start + n) > _len){ return {}; }
		return from(&_data[start], n);
	}

	constexpr
	static slice from(T* ptr, isize len){
		slice s;
		s._data = ptr;
		s._len = max(isize(0), len);
		return s;
	}

	/* C++ Iterator Insanity */
	constexpr auto begin(){ return cpp_iter::Contigous_Memory_Iterator<T>(&_data[0]); }
	constexpr auto end(){ return cpp_iter::Contigous_Memory_Iterator<T>(&_data[_len]); }
	class Slice_Index_Iterator {
		T* data; isize len;
	public:
		constexpr auto begin(){ return cpp_iter::Indexed_Contigous_Memory_Iterator<T>(&data[0]); }
		constexpr auto end(){ return cpp_iter::Indexed_Contigous_Memory_Iterator<T>(&data[len]); }
		Slice_Index_Iterator(T* data, isize len) : data(data), len(len){}
	};
	constexpr auto index_iter(){ return Slice_Index_Iterator(_data, _len); }

};

/* ---------------- UTF-8 Support ---------------- */
namespace utf8 {
constexpr inline i32 RANGE1 = 0x7f;
constexpr inline i32 RANGE2 = 0x7ff;
constexpr inline i32 RANGE3 = 0xffff;
constexpr inline i32 RANGE4 = 0x10ffff;

struct Encode_Result {
	byte bytes[4];
	i8 len;
};

struct Decode_Result {
	rune codepoint;
	i8 len;
};

constexpr inline rune ERROR = 0xfffd;

constexpr inline Encode_Result error_rune_encoded = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 3,
};

constexpr inline rune SURROGATE1 = 0xd800;
constexpr rune SURROGATE2 = 0xdfff;

constexpr inline rune MASK2 = 0x1f; /* 0001_1111 */
constexpr inline rune MASK3 = 0x0f; /* 0000_1111 */
constexpr inline rune MASK4 = 0x07; /* 0000_0111 */

constexpr inline rune MASKX  = 0x3f; /* 0011_1111 */

constexpr inline rune SIZE2 = 0xc0; /* 110x_xxxx */
constexpr inline rune SIZE3 = 0xe0; /* 1110_xxxx */
constexpr inline rune SIZE4 = 0xf0; /* 1111_0xxx */

constexpr inline rune CONT = 0x80; /* 10xx_xxxx */

constexpr inline rune CONTINUATION1  = 0x80;
constexpr inline rune CONTINUATION2  = 0xbf;

static inline
bool is_continuation_byte(rune c){
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

constexpr inline Decode_Result DECODE_ERROR = { .codepoint = ERROR, .len = 0 };

static inline
Encode_Result encode(rune c){
	Encode_Result res = {{0}, 0};

	if((c >= CONTINUATION1 && c <= CONTINUATION2)
		|| (c >= SURROGATE1 && c <= SURROGATE2)
		|| (c > RANGE4))
	{
		return error_rune_encoded;
	}

	if(c <= RANGE1){
		res.len = 1;
		res.bytes[0] = c;
	}
	else if(c <= RANGE2){
		res.len = 2;
		res.bytes[0] = SIZE2 | ((c >> 6) & MASK2);
		res.bytes[1] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE3){
		res.len = 3;
		res.bytes[0] = SIZE3 | ((c >> 12) & MASK3);
		res.bytes[1] = CONT  | ((c >> 6) & MASKX);
		res.bytes[2] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE4){
		res.len = 4;
		res.bytes[0] = SIZE4 | ((c >> 18) & MASK4);
		res.bytes[1] = CONT  | ((c >> 12) & MASKX);
		res.bytes[2] = CONT  | ((c >> 6)  & MASKX);
		res.bytes[3] = CONT  | ((c >> 0)  & MASKX);
	}
	return res;
}

static inline
Decode_Result utf8_decode(byte const* buf, isize len){
	Decode_Result res = {0, 0};
	if(buf == nullptr || len <= 0){ return DECODE_ERROR; }

	u8 first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~MASK2) == SIZE2 && len >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & MASK2) << 6;
		res.codepoint |= (buf[1] & MASKX) << 0;
	}
	else if ((first & ~MASK3) == SIZE3 && len >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & MASK3) << 12;
		res.codepoint |= (buf[1] & MASKX) << 6;
		res.codepoint |= (buf[2] & MASKX) << 0;
	}
	else if ((first & ~MASK4) == SIZE4 && len >= 4){
		res.len = 4;
		res.codepoint |= (buf[0] & MASK4) << 18;
		res.codepoint |= (buf[1] & MASKX) << 12;
		res.codepoint |= (buf[2] & MASKX) << 6;
		res.codepoint |= (buf[3] & MASKX) << 0;
	}
	else {
		return DECODE_ERROR;
	}

	// Validation
	if(res.codepoint >= SURROGATE1 && res.codepoint <= SURROGATE2){
		return DECODE_ERROR;
	}
	if(res.len > 1 && (buf[1] < CONTINUATION1 || buf[1] > CONTINUATION2)){
		return DECODE_ERROR;
	}
	if(res.len > 2 && (buf[2] < CONTINUATION1 || buf[2] > CONTINUATION2)){
		return DECODE_ERROR;
	}
	if(res.len > 3 && (buf[3] < CONTINUATION1 || buf[3] > CONTINUATION2)){
		return DECODE_ERROR;
	}

	return res;
}

struct Iterator {
	byte const* data;
	isize len;
	isize current;

	bool done() const {
		return current < 0 || current >= len;
	}

	Decode_Result next(){
		if(current >= len){ return {0, 0}; }

		Decode_Result res = utf8_decode(&data[current], len);

		if(res.codepoint == DECODE_ERROR.codepoint){
			res.len += 1;
		}

		current += res.len;

		return res;
	}

	Decode_Result prev(){
		if(current < 0){ return {0, 0}; }

		current -= 1;
		while(is_continuation_byte(data[current])){
			current -= 1;
		}

		Decode_Result res = utf8_decode(&data[current], len - current);
		return res;
	}

	void reset(){
		current = 0;
	}

	static Iterator from(byte const* data, isize cur, isize len){
		Iterator it;
		it.data = data;
		it.len = len;
		it.current = cur;
		return it;
	}
};
}

/* ---------------- Strings ---------------- */
static inline constexpr
isize cstring_len(cstring cstr){
	constexpr isize CSTR_MAX_LENGTH = (~(u32)0) >> 1;
	isize size = 0;
	for(isize i = 0; i < CSTR_MAX_LENGTH && cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

struct string {
private:
	isize _len = 0;
	union {
		byte const * _data = nullptr;
		char const * _cdata; /* NOTE: Used to do type punning because C++ prohibits reinterpret_cast on constexpr */
	};

	static constexpr i32 MAX_CUTSET_LEN = 64;
public:
	constexpr
	auto len() const { return _len; }

	constexpr
	auto raw_data() const { return _data; }

	constexpr
	bool empty() const {
		return _len == 0 ;
	}

	auto iterator() const {
		return utf8::Iterator::from(_data, 0, _len);
	}

	auto reverse_iterator() const {
		return utf8::Iterator::from(_data, _len, _len);
	}

	isize rune_count() const {
		isize count = 0;
		for(auto it = iterator(); !it.done(); ){
			it.next();
			count += 1;
		}
		return count;
	}

	isize rune_offset(isize n) const {
		auto it = iterator();

		isize seen_runes = 0;
		bool found = 0;

		while(!it.done()){
			it.next();
			seen_runes += 1;
			if(n == seen_runes){
				found = true;
				break;
			}
		}

		return found ? it.current : -1;
	}

	constexpr
	byte operator[](isize idx) const {
		bounds_check(idx > 0 && idx < _len, "Out of bounds access");
		return _data[idx];
	}

	string sub(isize start, isize byte_count) const {
		if(start < 0 || byte_count < 0 || (start + byte_count) > _len){ return {}; }

		auto sub = from_bytes(&_data[start], byte_count);

		return sub;
	}

	string trim_leading(string cutset) const {
		rune set[MAX_CUTSET_LEN] = {0};
		isize set_len = 0;
		isize cut_after = 0;

		/* Decode Cutset */ {
			auto iter = cutset.iterator();

			isize i = 0;
			while(!iter.done()){
				auto [c, _] = iter.next();
				set[i] = c;
				i += 1;
			}
			set_len = i;
		}

		/* Strip Cutset */ {
			auto iter = this->iterator();

			while(!iter.done()){
				bool to_be_cut = false;
				auto [c, n] = iter.next();

				for(isize i = 0; i < set_len; i += 1){
					if(set[i] == c){
						to_be_cut = true;
						break;
					}
				}

				if(to_be_cut){
					cut_after += n;
				}
				else {
					break; /* Reached first rune that isn't in cutset */
				}
			}
		}

		return this->sub(cut_after, _len - cut_after);
	}

	string trim_trailing(string cutset) const {
		rune set[MAX_CUTSET_LEN] = {0};
		isize set_len = 0;
		isize cut_until = _len;

		/* Decode Cutset */ {
			auto iter = cutset.iterator();

			isize i = 0;
			while(!iter.done()){
				auto [c, _] = iter.next();
				set[i] = c;
				i += 1;
			}
			set_len = i;
		}

		/* Strip Cutset */ {
			auto iter = this->reverse_iterator();

			do {
				bool to_be_cut = false;
				auto [c, n] = iter.prev();

				for(isize i = 0; i < set_len; i += 1){
					if(set[i] == c){
						to_be_cut = true;
						break;
					}
				}

				if(to_be_cut){
					cut_until -= n;
				}
				else {
					break; /* Reached first rune that isn't in cutset */
				}

			} while(!iter.done());
		}

		return sub(0, cut_until);
	}

	string trim(string cutset) const {
		return this->trim_leading(cutset).trim_trailing(cutset);
	}

	string trim_whitespace() const {
		return trim(" \t\r\n\v");
	}

	static
	string from_bytes(byte const * p, isize length){
		string s;
		s._data = p;
		s._len = length;
		return s;
	}

	bool operator==(string rhs) {
		if(_len != rhs._len){ return false; }
		return mem::compare(_data, rhs._data, _len) == 0;
	}

	bool operator!=(string rhs) {
		if(_len != rhs._len){ return true; }
		return mem::compare(_data, rhs._data, _len) != 0;
	}

	constexpr string(){}
	constexpr string(cstring cs) : _len(cstring_len(cs)), _cdata(cs) {}
};

#undef MAX_CUTSET_LEN

/* ---------------- Spinlock ---------------- */
namespace atomic {
struct Spinlock {
	std::atomic_int _state = 0;

	constexpr static int locked = 1;
	constexpr static int unlocked = 0;

	void acquire(){
		for(;;){
			if(!atomic::exchange(&_state, Spinlock::locked, Memory_Order::acquire)){
				break;
			}
			/* Busy wait while locked */
			while(atomic::load(&_state, Memory_Order::relaxed));
		}
	}

	bool try_acquire(){
		return !atomic::exchange(&_state, Spinlock::locked, Memory_Order::acquire);
	}

	void release(){
		atomic::store(&_state, Spinlock::unlocked, Memory_Order::release);
	}
};
}

/* ---------------- Allocator Interface ---------------- */
namespace mem {
enum struct Allocator_Mode : u8 {
	alloc_non_zero, alloc, resize, free, free_all, query,
};

// Allocator capabilities
constexpr inline u32 can_alloc_any_size  = 1 << 0;
constexpr inline u32 can_alloc_any_align = 1 << 1;
constexpr inline u32 can_free_any_order  = 1 << 2;
constexpr inline u32 can_resize          = 1 << 3;
constexpr inline u32 can_free_all        = 1 << 4;

// Allocator_Error's are meant to be thrown, that is, they are exceptional
// conditions. Failing to resize an allocation or not supporting `free_all`
// should be communicated through the return address or by querying the
// allocator's capabilities.
enum struct Allocator_Error : u8 {
	none = 0,
	out_of_memory,
	bad_align,
	pointer_not_owned, /* Mostly used by tracking allocators, it's usually not worth throwing it in release builds */
};

// Allocator function, returns a error value
using Allocator_Func = void* (*)(
    void *impl, Allocator_Mode mode, void *ptr, isize old_size, isize size,
    isize align, Source_Location const& source_location);

// Allocator interface
struct Allocator {
	void* _impl = nullptr;
	Allocator_Func _func = nullptr;

	using Mode = Allocator_Mode;

	// Allocate chunk of aligned memory (zero-initialized)
	auto alloc(isize size, isize align, caller_location){
		return _func(_impl, Mode::alloc, nullptr, 0, size, align, source_location);
	}

	// Allocate chunk of aligned memory (not initialized)
	auto alloc_non_zero(isize size, isize align, caller_location){
		return _func(_impl, Mode::alloc_non_zero, nullptr, 0, size, align, source_location);
	}

	// Mark pointer that belongs to allocator as free
	void free(void* ptr, isize old_size = 0, caller_location) noexcept {
		_func(_impl, Mode::free, ptr, old_size, 0, 0, source_location);
	}

	// Mark all memory belonging to allocator as free
	void free_all(caller_location) noexcept {
		_func(_impl, Mode::free_all, nullptr, 0, 0, 0, source_location);
	}

	// Attempt to resize allocation *in-place*. Returns null on failure, on sucess gives back the original pointer.
	auto resize(void* ptr, isize new_size, isize old_size, caller_location) noexcept {
		return _func(_impl, Mode::resize, ptr, old_size, new_size, 0, source_location);
	}

	template<typename T>
	T* make(caller_location){
		auto res = alloc(sizeof(T), alignof(T), source_location);
		return (T*) res;
	}

	template<typename T>
	slice<T> make_slice(isize count, caller_location){
		auto res = (T*) alloc(sizeof(T) * count, alignof(T), source_location);
		return slice<T>::from(res, count);
	}

	template<typename T>
	void destroy(T* ptr, caller_location){
		free((void*)ptr, sizeof(T), source_location);
	}

	template<typename T>
	void destroy(slice<T> s, caller_location){
		free((void*)s.raw_data(), s.len() * sizeof(T), source_location);
	}

	void destroy(string s, caller_location){
		free((void*)s.raw_data(), s.len(), source_location);
	}

	static Allocator from(void* impl, Allocator_Func func) {
		Allocator a {impl, func};
		return a;
	}
};
}

/* ---------------- Arena ---------------- */
namespace mem {
struct Arena {
	byte* data = nullptr;
	isize offset = 0;
	isize cap = 0;
	void* last_allocation = nullptr;

	uintptr required_mem(uintptr cur, isize nbytes, isize align) const {
		if(!mem::valid_alignment(align)){
			throw Allocator_Error::bad_align;
		}
		uintptr aligned  = mem::align_forward(cur, align);
		uintptr padding  = (uintptr)(aligned - cur);
		uintptr required = padding + nbytes;
		return required;
	}

	void* alloc_non_zero(isize size, isize align){
		uintptr base = (uintptr)data;
		uintptr current = (uintptr)base + (uintptr)offset;

		uintptr available = (uintptr)cap - (current - base);
		uintptr required = required_mem(current, size, align);

		if(required > available){
			return nullptr;
		}

		offset += required;
		void* allocation = &data[offset - size];
		last_allocation = allocation;
		return allocation;
	}

	void* alloc(isize size, isize align){
		void* p = alloc_non_zero(size, align);
		if(p != nullptr){
			mem::set(p, 0, size);
		}
		return p;
	}

	void reset(){
		offset = 0;
		last_allocation = nullptr;
	}

	// Try to resize arena allocation in-place, returns nullptr if failed
	void* resize(void* p, isize size){
		if(p != last_allocation || size < 0 || p == nullptr){
			return nullptr;
		}

		uintptr base = (uintptr)data;
		uintptr last_offset = (uintptr)p - base;

		isize old_size = offset - last_offset;
		isize delta = size - old_size;

		if((offset + delta) < cap){
			offset += delta;
		}
		else {
			return nullptr;
		}

		return p;
	}

	static Arena from_bytes(slice<byte> b){
		Arena a = {
			.data = b.raw_data(),
			.offset = 0,
			.cap = b.len(),
			.last_allocation = nullptr,
		};
		return a;
	}

	Allocator allocator(); /* Defined below */
};

static inline void* _arena_allocator_func(
	void *impl,
	Allocator_Mode mode,
	void *ptr,
	[[maybe_unused]] isize old_size,
	isize size,
	isize align,
	[[maybe_unused]] caller_location
){
	auto arena = (Arena*)impl;
	switch (mode) {

	case Allocator_Mode::query: {
		u32 capabilities = can_alloc_any_size | can_alloc_any_align | can_free_all | can_resize;
		return (void*)(uintptr)capabilities;
	} break;

	case Allocator_Mode::alloc_non_zero: {
		void* p = arena->alloc_non_zero(size, align);
		if(!p){
			throw Allocator_Error::out_of_memory;
		}
		return p;
	} break;

	case Allocator_Mode::alloc: {
		void* p = arena->alloc(size, align);
		if(!p){
			throw Allocator_Error::out_of_memory;
		}
		return p;
	} break;

	case Allocator_Mode::resize: {
		void* p = arena->resize(ptr, size);
		if(!p){
			return nullptr;
		}
		return p;
	} break;

	case Allocator_Mode::free: {
		/* Nothing */
	} break;

	case Allocator_Mode::free_all: {
		arena->reset();
	} break;
	}

	return nullptr;
}

inline Allocator Arena::allocator(){
	return Allocator::from(
		(void*)this,
		_arena_allocator_func
	);
}
}

/* --------------- Null Allocator --------------- */
namespace mem {
static inline void* _null_allocator_func(void *, Allocator_Mode, void *, isize, isize, isize, Source_Location const&){
	throw Allocator_Error::out_of_memory;
}

static Allocator null_allocator(){
	return Allocator::from(nullptr, _null_allocator_func);
}
}

/* ---------------- Heap Allocator ---------------- */
namespace mem {
static inline void* _heap_allocator_func(
	[[maybe_unused]] void *impl,
	Allocator_Mode mode,
	void *ptr,
	[[maybe_unused]] isize old_size,
	isize size,
	isize align,
	[[maybe_unused]] caller_location
){
	try {
		switch (mode) {
		case Allocator_Mode::query: {
			u32 capabilities = can_alloc_any_size | can_alloc_any_align | can_free_any_order;
			return (void*)(uintptr)capabilities;
		} break;

		case Allocator_Mode::alloc_non_zero: {
			byte* p = new (std::align_val_t(align)) byte[size];
			return (void*) p;
		} break;

		case Allocator_Mode::alloc: {
			byte* p = new (std::align_val_t(align)) byte[size];
			if(p){
				mem::set(p, 0, size);
			}
			return (void*) p;
		} break;

		case Allocator_Mode::resize: {
			return nullptr;
		}

		case Allocator_Mode::free: {
			delete [] (byte*)ptr;
		} break;

		case Allocator_Mode::free_all:
			return nullptr;
		}

	} catch(std::bad_alloc const&){
		throw Allocator_Error::out_of_memory;
	}

	return nullptr;
}

static inline Allocator heap_allocator(){
	return Allocator::from(nullptr, _heap_allocator_func);
}
}

/* ---------------- Dynamic Array ---------------- */
template<typename T>
struct Dynamic_Array {
	T* data = nullptr;
	isize length = 0;
	isize capacity = 0;
	mem::Allocator allocator;

	auto len() const { return length; }

	auto cap() const { return capacity; }

	void resize(isize new_cap){
		isize new_size = new_cap * sizeof(T);
		isize old_size = length * sizeof(T);

		void* new_data = allocator.resize((void*) data, new_size, old_size);
		if(new_data == nullptr){
			new_data = allocator.alloc(new_size, alignof(T));
		}

		mem::copy(new_data, data, min(new_size, old_size));
		allocator.free(data, old_size);

		data     = (T*)new_data;
		capacity = new_cap;
		length   = min(length, new_cap);
	}

	void append(T val){
		if(length >= capacity){
			resize(max(isize(16), length * 2));
		}
		data[length] = val;
		length += 1;
	}

	void pop(){
		length = min(length - 1, isize(0));
	}

	void insert(isize idx, T val){
		bounds_check(idx >= 0 && idx <= length, "Index out of bounds");
		if(length >= capacity){
			resize(max(isize(16), length * 2));
		}
		mem::copy(&data[idx+1], &data[idx], sizeof(T) * (length - idx));
		data[idx] = val;
		length += 1;
	}

	void remove(isize idx){
		bounds_check(idx >= 0 && idx <= length, "Index out of bounds");
		if(length <= 0){ return; }
		mem::copy(&data[idx], &data[idx+1], sizeof(T) * (length - idx));
		length -= 1;
	}

	T& operator[](isize idx){
		bounds_check(idx >= 0 && idx <= length, "Index out of bounds");
		return data[idx];
	}

	T const& operator[](isize idx) const {
		bounds_check(idx >= 0 && idx <= length, "Index out of bounds");
		return data[idx];
	}

	slice<T> sub(){
		auto s = slice<T>::from(&data[0], length);
		return s;
	}

	slice<T> sub(isize idx, isize len){
		auto s = slice<T>::from(&data[idx], len);
		return s;
	}

	void deinit(){
		allocator.free(data);
		data = nullptr;
		capacity = 0;
	}

	static Dynamic_Array<T> from(mem::Allocator allocator, isize initial_cap = 16){
		Dynamic_Array<T> arr;
		arr.allocator = allocator;
		if(initial_cap > 0){
			arr.data = (T*)allocator.alloc(sizeof(T) * initial_cap, alignof(T));
			arr.capacity = initial_cap;
		}
		return arr;
	}

	/* C++ Iterator Insanity */
	auto begin(){ return sub().begin(); }
	auto end(){ return sub().end(); }
	auto index_iter() { return sub().index_iter(); }
};

template<typename T>
void destroy(Dynamic_Array<T>* arr){
	arr->deinit();
}

