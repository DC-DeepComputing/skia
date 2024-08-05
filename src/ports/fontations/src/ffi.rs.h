#pragma once
#include "src/ports/fontations/src/skpath_bridge.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#if defined(_WIN32)
#include <basetsd.h>
#else
#include <sys/types.h>
#endif

namespace rust {
inline namespace cxxbridge1 {
// #include "rust/cxx.h"

#ifndef CXXBRIDGE1_PANIC
#define CXXBRIDGE1_PANIC
template <typename Exception>
void panic [[noreturn]] (const char *msg);
#endif // CXXBRIDGE1_PANIC

struct unsafe_bitcopy_t;

namespace {
template <typename T>
class impl;
} // namespace

template <typename T>
::std::size_t size_of();
template <typename T>
::std::size_t align_of();

#ifndef CXXBRIDGE1_RUST_STRING
#define CXXBRIDGE1_RUST_STRING
class String final {
public:
  String() noexcept;
  String(const String &) noexcept;
  String(String &&) noexcept;
  ~String() noexcept;

  String(const std::string &);
  String(const char *);
  String(const char *, std::size_t);
  String(const char16_t *);
  String(const char16_t *, std::size_t);

  static String lossy(const std::string &) noexcept;
  static String lossy(const char *) noexcept;
  static String lossy(const char *, std::size_t) noexcept;
  static String lossy(const char16_t *) noexcept;
  static String lossy(const char16_t *, std::size_t) noexcept;

  String &operator=(const String &) &noexcept;
  String &operator=(String &&) &noexcept;

  explicit operator std::string() const;

  const char *data() const noexcept;
  std::size_t size() const noexcept;
  std::size_t length() const noexcept;
  bool empty() const noexcept;

  const char *c_str() noexcept;

  std::size_t capacity() const noexcept;
  void reserve(size_t new_cap) noexcept;

  using iterator = char *;
  iterator begin() noexcept;
  iterator end() noexcept;

  using const_iterator = const char *;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  bool operator==(const String &) const noexcept;
  bool operator!=(const String &) const noexcept;
  bool operator<(const String &) const noexcept;
  bool operator<=(const String &) const noexcept;
  bool operator>(const String &) const noexcept;
  bool operator>=(const String &) const noexcept;

  void swap(String &) noexcept;

  String(unsafe_bitcopy_t, const String &) noexcept;

private:
  struct lossy_t;
  String(lossy_t, const char *, std::size_t) noexcept;
  String(lossy_t, const char16_t *, std::size_t) noexcept;
  friend void swap(String &lhs, String &rhs) noexcept { lhs.swap(rhs); }

  std::array<std::uintptr_t, 3> repr;
};
#endif // CXXBRIDGE1_RUST_STRING

#ifndef CXXBRIDGE1_RUST_SLICE
#define CXXBRIDGE1_RUST_SLICE
namespace detail {
template <bool>
struct copy_assignable_if {};

template <>
struct copy_assignable_if<false> {
  copy_assignable_if() noexcept = default;
  copy_assignable_if(const copy_assignable_if &) noexcept = default;
  copy_assignable_if &operator=(const copy_assignable_if &) &noexcept = delete;
  copy_assignable_if &operator=(copy_assignable_if &&) &noexcept = default;
};
} // namespace detail

template <typename T>
class Slice final
    : private detail::copy_assignable_if<std::is_const<T>::value> {
public:
  using value_type = T;

  Slice() noexcept;
  Slice(T *, std::size_t count) noexcept;

  Slice &operator=(const Slice<T> &) &noexcept = default;
  Slice &operator=(Slice<T> &&) &noexcept = default;

  T *data() const noexcept;
  std::size_t size() const noexcept;
  std::size_t length() const noexcept;
  bool empty() const noexcept;

  T &operator[](std::size_t n) const noexcept;
  T &at(std::size_t n) const;
  T &front() const noexcept;
  T &back() const noexcept;

  Slice(const Slice<T> &) noexcept = default;
  ~Slice() noexcept = default;

  class iterator;
  iterator begin() const noexcept;
  iterator end() const noexcept;

  void swap(Slice &) noexcept;

private:
  class uninit;
  Slice(uninit) noexcept;
  friend impl<Slice>;
  friend void sliceInit(void *, const void *, std::size_t) noexcept;
  friend void *slicePtr(const void *) noexcept;
  friend std::size_t sliceLen(const void *) noexcept;

  std::array<std::uintptr_t, 2> repr;
};

template <typename T>
class Slice<T>::iterator final {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = typename std::add_pointer<T>::type;
  using reference = typename std::add_lvalue_reference<T>::type;

  reference operator*() const noexcept;
  pointer operator->() const noexcept;
  reference operator[](difference_type) const noexcept;

  iterator &operator++() noexcept;
  iterator operator++(int) noexcept;
  iterator &operator--() noexcept;
  iterator operator--(int) noexcept;

  iterator &operator+=(difference_type) noexcept;
  iterator &operator-=(difference_type) noexcept;
  iterator operator+(difference_type) const noexcept;
  iterator operator-(difference_type) const noexcept;
  difference_type operator-(const iterator &) const noexcept;

  bool operator==(const iterator &) const noexcept;
  bool operator!=(const iterator &) const noexcept;
  bool operator<(const iterator &) const noexcept;
  bool operator<=(const iterator &) const noexcept;
  bool operator>(const iterator &) const noexcept;
  bool operator>=(const iterator &) const noexcept;

private:
  friend class Slice;
  void *pos;
  std::size_t stride;
};

template <typename T>
Slice<T>::Slice() noexcept {
  sliceInit(this, reinterpret_cast<void *>(align_of<T>()), 0);
}

template <typename T>
Slice<T>::Slice(T *s, std::size_t count) noexcept {
  assert(s != nullptr || count == 0);
  sliceInit(this,
            s == nullptr && count == 0
                ? reinterpret_cast<void *>(align_of<T>())
                : const_cast<typename std::remove_const<T>::type *>(s),
            count);
}

template <typename T>
T *Slice<T>::data() const noexcept {
  return reinterpret_cast<T *>(slicePtr(this));
}

template <typename T>
std::size_t Slice<T>::size() const noexcept {
  return sliceLen(this);
}

template <typename T>
std::size_t Slice<T>::length() const noexcept {
  return this->size();
}

template <typename T>
bool Slice<T>::empty() const noexcept {
  return this->size() == 0;
}

template <typename T>
T &Slice<T>::operator[](std::size_t n) const noexcept {
  assert(n < this->size());
  auto ptr = static_cast<char *>(slicePtr(this)) + size_of<T>() * n;
  return *reinterpret_cast<T *>(ptr);
}

template <typename T>
T &Slice<T>::at(std::size_t n) const {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Slice index out of range");
  }
  return (*this)[n];
}

template <typename T>
T &Slice<T>::front() const noexcept {
  assert(!this->empty());
  return (*this)[0];
}

template <typename T>
T &Slice<T>::back() const noexcept {
  assert(!this->empty());
  return (*this)[this->size() - 1];
}

template <typename T>
typename Slice<T>::iterator::reference
Slice<T>::iterator::operator*() const noexcept {
  return *static_cast<T *>(this->pos);
}

template <typename T>
typename Slice<T>::iterator::pointer
Slice<T>::iterator::operator->() const noexcept {
  return static_cast<T *>(this->pos);
}

template <typename T>
typename Slice<T>::iterator::reference Slice<T>::iterator::operator[](
    typename Slice<T>::iterator::difference_type n) const noexcept {
  auto ptr = static_cast<char *>(this->pos) + this->stride * n;
  return *reinterpret_cast<T *>(ptr);
}

template <typename T>
typename Slice<T>::iterator &Slice<T>::iterator::operator++() noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return *this;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::iterator::operator++(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) + this->stride;
  return ret;
}

template <typename T>
typename Slice<T>::iterator &Slice<T>::iterator::operator--() noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return *this;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::iterator::operator--(int) noexcept {
  auto ret = iterator(*this);
  this->pos = static_cast<char *>(this->pos) - this->stride;
  return ret;
}

template <typename T>
typename Slice<T>::iterator &Slice<T>::iterator::operator+=(
    typename Slice<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) + this->stride * n;
  return *this;
}

template <typename T>
typename Slice<T>::iterator &Slice<T>::iterator::operator-=(
    typename Slice<T>::iterator::difference_type n) noexcept {
  this->pos = static_cast<char *>(this->pos) - this->stride * n;
  return *this;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::iterator::operator+(
    typename Slice<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) + this->stride * n;
  return ret;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::iterator::operator-(
    typename Slice<T>::iterator::difference_type n) const noexcept {
  auto ret = iterator(*this);
  ret.pos = static_cast<char *>(this->pos) - this->stride * n;
  return ret;
}

template <typename T>
typename Slice<T>::iterator::difference_type
Slice<T>::iterator::operator-(const iterator &other) const noexcept {
  auto diff = std::distance(static_cast<char *>(other.pos),
                            static_cast<char *>(this->pos));
  return diff / this->stride;
}

template <typename T>
bool Slice<T>::iterator::operator==(const iterator &other) const noexcept {
  return this->pos == other.pos;
}

template <typename T>
bool Slice<T>::iterator::operator!=(const iterator &other) const noexcept {
  return this->pos != other.pos;
}

template <typename T>
bool Slice<T>::iterator::operator<(const iterator &other) const noexcept {
  return this->pos < other.pos;
}

template <typename T>
bool Slice<T>::iterator::operator<=(const iterator &other) const noexcept {
  return this->pos <= other.pos;
}

template <typename T>
bool Slice<T>::iterator::operator>(const iterator &other) const noexcept {
  return this->pos > other.pos;
}

template <typename T>
bool Slice<T>::iterator::operator>=(const iterator &other) const noexcept {
  return this->pos >= other.pos;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::begin() const noexcept {
  iterator it;
  it.pos = slicePtr(this);
  it.stride = size_of<T>();
  return it;
}

template <typename T>
typename Slice<T>::iterator Slice<T>::end() const noexcept {
  iterator it = this->begin();
  it.pos = static_cast<char *>(it.pos) + it.stride * this->size();
  return it;
}

template <typename T>
void Slice<T>::swap(Slice &rhs) noexcept {
  std::swap(*this, rhs);
}
#endif // CXXBRIDGE1_RUST_SLICE

#ifndef CXXBRIDGE1_RUST_BOX
#define CXXBRIDGE1_RUST_BOX
template <typename T>
class Box final {
public:
  using element_type = T;
  using const_pointer =
      typename std::add_pointer<typename std::add_const<T>::type>::type;
  using pointer = typename std::add_pointer<T>::type;

  Box() = delete;
  Box(Box &&) noexcept;
  ~Box() noexcept;

  explicit Box(const T &);
  explicit Box(T &&);

  Box &operator=(Box &&) &noexcept;

  const T *operator->() const noexcept;
  const T &operator*() const noexcept;
  T *operator->() noexcept;
  T &operator*() noexcept;

  template <typename... Fields>
  static Box in_place(Fields &&...);

  void swap(Box &) noexcept;

  static Box from_raw(T *) noexcept;

  T *into_raw() noexcept;

  /* Deprecated */ using value_type = element_type;

private:
  class uninit;
  class allocation;
  Box(uninit) noexcept;
  void drop() noexcept;

  friend void swap(Box &lhs, Box &rhs) noexcept { lhs.swap(rhs); }

  T *ptr;
};

template <typename T>
class Box<T>::uninit {};

template <typename T>
class Box<T>::allocation {
  static T *alloc() noexcept;
  static void dealloc(T *) noexcept;

public:
  allocation() noexcept : ptr(alloc()) {}
  ~allocation() noexcept {
    if (this->ptr) {
      dealloc(this->ptr);
    }
  }
  T *ptr;
};

template <typename T>
Box<T>::Box(Box &&other) noexcept : ptr(other.ptr) {
  other.ptr = nullptr;
}

template <typename T>
Box<T>::Box(const T &val) {
  allocation alloc;
  ::new (alloc.ptr) T(val);
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::Box(T &&val) {
  allocation alloc;
  ::new (alloc.ptr) T(std::move(val));
  this->ptr = alloc.ptr;
  alloc.ptr = nullptr;
}

template <typename T>
Box<T>::~Box() noexcept {
  if (this->ptr) {
    this->drop();
  }
}

template <typename T>
Box<T> &Box<T>::operator=(Box &&other) &noexcept {
  if (this->ptr) {
    this->drop();
  }
  this->ptr = other.ptr;
  other.ptr = nullptr;
  return *this;
}

template <typename T>
const T *Box<T>::operator->() const noexcept {
  return this->ptr;
}

template <typename T>
const T &Box<T>::operator*() const noexcept {
  return *this->ptr;
}

template <typename T>
T *Box<T>::operator->() noexcept {
  return this->ptr;
}

template <typename T>
T &Box<T>::operator*() noexcept {
  return *this->ptr;
}

template <typename T>
template <typename... Fields>
Box<T> Box<T>::in_place(Fields &&...fields) {
  allocation alloc;
  auto ptr = alloc.ptr;
  ::new (ptr) T{std::forward<Fields>(fields)...};
  alloc.ptr = nullptr;
  return from_raw(ptr);
}

template <typename T>
void Box<T>::swap(Box &rhs) noexcept {
  using std::swap;
  swap(this->ptr, rhs.ptr);
}

template <typename T>
Box<T> Box<T>::from_raw(T *raw) noexcept {
  Box box = uninit{};
  box.ptr = raw;
  return box;
}

template <typename T>
T *Box<T>::into_raw() noexcept {
  T *raw = this->ptr;
  this->ptr = nullptr;
  return raw;
}

template <typename T>
Box<T>::Box(uninit) noexcept {}
#endif // CXXBRIDGE1_RUST_BOX

#ifndef CXXBRIDGE1_RUST_BITCOPY_T
#define CXXBRIDGE1_RUST_BITCOPY_T
struct unsafe_bitcopy_t final {
  explicit unsafe_bitcopy_t() = default;
};
#endif // CXXBRIDGE1_RUST_BITCOPY_T

#ifndef CXXBRIDGE1_RUST_VEC
#define CXXBRIDGE1_RUST_VEC
template <typename T>
class Vec final {
public:
  using value_type = T;

  Vec() noexcept;
  Vec(std::initializer_list<T>);
  Vec(const Vec &);
  Vec(Vec &&) noexcept;
  ~Vec() noexcept;

  Vec &operator=(Vec &&) &noexcept;
  Vec &operator=(const Vec &) &;

  std::size_t size() const noexcept;
  bool empty() const noexcept;
  const T *data() const noexcept;
  T *data() noexcept;
  std::size_t capacity() const noexcept;

  const T &operator[](std::size_t n) const noexcept;
  const T &at(std::size_t n) const;
  const T &front() const noexcept;
  const T &back() const noexcept;

  T &operator[](std::size_t n) noexcept;
  T &at(std::size_t n);
  T &front() noexcept;
  T &back() noexcept;

  void reserve(std::size_t new_cap);
  void push_back(const T &value);
  void push_back(T &&value);
  template <typename... Args>
  void emplace_back(Args &&...args);
  void truncate(std::size_t len);
  void clear();

  using iterator = typename Slice<T>::iterator;
  iterator begin() noexcept;
  iterator end() noexcept;

  using const_iterator = typename Slice<const T>::iterator;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  void swap(Vec &) noexcept;

  Vec(unsafe_bitcopy_t, const Vec &) noexcept;

private:
  void reserve_total(std::size_t new_cap) noexcept;
  void set_len(std::size_t len) noexcept;
  void drop() noexcept;

  friend void swap(Vec &lhs, Vec &rhs) noexcept { lhs.swap(rhs); }

  std::array<std::uintptr_t, 3> repr;
};

template <typename T>
Vec<T>::Vec(std::initializer_list<T> init) : Vec{} {
  this->reserve_total(init.size());
  std::move(init.begin(), init.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(const Vec &other) : Vec() {
  this->reserve_total(other.size());
  std::copy(other.begin(), other.end(), std::back_inserter(*this));
}

template <typename T>
Vec<T>::Vec(Vec &&other) noexcept : repr(other.repr) {
  new (&other) Vec();
}

template <typename T>
Vec<T>::~Vec() noexcept {
  this->drop();
}

template <typename T>
Vec<T> &Vec<T>::operator=(Vec &&other) &noexcept {
  this->drop();
  this->repr = other.repr;
  new (&other) Vec();
  return *this;
}

template <typename T>
Vec<T> &Vec<T>::operator=(const Vec &other) & {
  if (this != &other) {
    this->drop();
    new (this) Vec(other);
  }
  return *this;
}

template <typename T>
bool Vec<T>::empty() const noexcept {
  return this->size() == 0;
}

template <typename T>
T *Vec<T>::data() noexcept {
  return const_cast<T *>(const_cast<const Vec<T> *>(this)->data());
}

template <typename T>
const T &Vec<T>::operator[](std::size_t n) const noexcept {
  assert(n < this->size());
  auto data = reinterpret_cast<const char *>(this->data());
  return *reinterpret_cast<const T *>(data + n * size_of<T>());
}

template <typename T>
const T &Vec<T>::at(std::size_t n) const {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
const T &Vec<T>::front() const noexcept {
  assert(!this->empty());
  return (*this)[0];
}

template <typename T>
const T &Vec<T>::back() const noexcept {
  assert(!this->empty());
  return (*this)[this->size() - 1];
}

template <typename T>
T &Vec<T>::operator[](std::size_t n) noexcept {
  assert(n < this->size());
  auto data = reinterpret_cast<char *>(this->data());
  return *reinterpret_cast<T *>(data + n * size_of<T>());
}

template <typename T>
T &Vec<T>::at(std::size_t n) {
  if (n >= this->size()) {
    panic<std::out_of_range>("rust::Vec index out of range");
  }
  return (*this)[n];
}

template <typename T>
T &Vec<T>::front() noexcept {
  assert(!this->empty());
  return (*this)[0];
}

template <typename T>
T &Vec<T>::back() noexcept {
  assert(!this->empty());
  return (*this)[this->size() - 1];
}

template <typename T>
void Vec<T>::reserve(std::size_t new_cap) {
  this->reserve_total(new_cap);
}

template <typename T>
void Vec<T>::push_back(const T &value) {
  this->emplace_back(value);
}

template <typename T>
void Vec<T>::push_back(T &&value) {
  this->emplace_back(std::move(value));
}

template <typename T>
template <typename... Args>
void Vec<T>::emplace_back(Args &&...args) {
  auto size = this->size();
  this->reserve_total(size + 1);
  ::new (reinterpret_cast<T *>(reinterpret_cast<char *>(this->data()) +
                               size * size_of<T>()))
      T(std::forward<Args>(args)...);
  this->set_len(size + 1);
}

template <typename T>
void Vec<T>::clear() {
  this->truncate(0);
}

template <typename T>
typename Vec<T>::iterator Vec<T>::begin() noexcept {
  return Slice<T>(this->data(), this->size()).begin();
}

template <typename T>
typename Vec<T>::iterator Vec<T>::end() noexcept {
  return Slice<T>(this->data(), this->size()).end();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::begin() const noexcept {
  return this->cbegin();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::end() const noexcept {
  return this->cend();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cbegin() const noexcept {
  return Slice<const T>(this->data(), this->size()).begin();
}

template <typename T>
typename Vec<T>::const_iterator Vec<T>::cend() const noexcept {
  return Slice<const T>(this->data(), this->size()).end();
}

template <typename T>
void Vec<T>::swap(Vec &rhs) noexcept {
  using std::swap;
  swap(this->repr, rhs.repr);
}

template <typename T>
Vec<T>::Vec(unsafe_bitcopy_t, const Vec &bits) noexcept : repr(bits.repr) {}
#endif // CXXBRIDGE1_RUST_VEC

#ifndef CXXBRIDGE1_RUST_ISIZE
#define CXXBRIDGE1_RUST_ISIZE
#if defined(_WIN32)
using isize = SSIZE_T;
#else
using isize = ssize_t;
#endif
#endif // CXXBRIDGE1_RUST_ISIZE

#ifndef CXXBRIDGE1_RUST_OPAQUE
#define CXXBRIDGE1_RUST_OPAQUE
class Opaque {
public:
  Opaque() = delete;
  Opaque(const Opaque &) = delete;
  ~Opaque() = delete;
};
#endif // CXXBRIDGE1_RUST_OPAQUE

#ifndef CXXBRIDGE1_IS_COMPLETE
#define CXXBRIDGE1_IS_COMPLETE
namespace detail {
namespace {
template <typename T, typename = std::size_t>
struct is_complete : std::false_type {};
template <typename T>
struct is_complete<T, decltype(sizeof(T))> : std::true_type {};
} // namespace
} // namespace detail
#endif // CXXBRIDGE1_IS_COMPLETE

#ifndef CXXBRIDGE1_LAYOUT
#define CXXBRIDGE1_LAYOUT
class layout {
  template <typename T>
  friend std::size_t size_of();
  template <typename T>
  friend std::size_t align_of();
  template <typename T>
  static typename std::enable_if<std::is_base_of<Opaque, T>::value,
                                 std::size_t>::type
  do_size_of() {
    return T::layout::size();
  }
  template <typename T>
  static typename std::enable_if<!std::is_base_of<Opaque, T>::value,
                                 std::size_t>::type
  do_size_of() {
    return sizeof(T);
  }
  template <typename T>
  static
      typename std::enable_if<detail::is_complete<T>::value, std::size_t>::type
      size_of() {
    return do_size_of<T>();
  }
  template <typename T>
  static typename std::enable_if<std::is_base_of<Opaque, T>::value,
                                 std::size_t>::type
  do_align_of() {
    return T::layout::align();
  }
  template <typename T>
  static typename std::enable_if<!std::is_base_of<Opaque, T>::value,
                                 std::size_t>::type
  do_align_of() {
    return alignof(T);
  }
  template <typename T>
  static
      typename std::enable_if<detail::is_complete<T>::value, std::size_t>::type
      align_of() {
    return do_align_of<T>();
  }
};

template <typename T>
std::size_t size_of() {
  return layout::size_of<T>();
}

template <typename T>
std::size_t align_of() {
  return layout::align_of<T>();
}
#endif // CXXBRIDGE1_LAYOUT
} // namespace cxxbridge1
} // namespace rust

namespace fontations_ffi {
  struct ColorStop;
  struct Metrics;
  struct BridgeLocalizedName;
  struct SkiaDesignCoordinate;
  struct BridgeScalerMetrics;
  struct PaletteOverride;
  struct ClipBox;
  struct Transform;
  struct FillLinearParams;
  struct FillRadialParams;
  struct FillSweepParams;
  struct BridgeFontStyle;
  struct BitmapMetrics;
  struct BridgeFontRef;
  struct BridgeOutlineCollection;
  struct BridgeMappingIndex;
  struct BridgeHintingInstance;
  struct BridgeBitmapGlyph;
  struct BridgeLocalizedStrings;
  struct BridgeNormalizedCoords;
  struct BridgeColorStops;
  using PathWrapper = ::fontations_ffi::PathWrapper;
  using AxisWrapper = ::fontations_ffi::AxisWrapper;
  using ColorPainterWrapper = ::fontations_ffi::ColorPainterWrapper;
}

namespace fontations_ffi {
#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$ColorStop
#define CXXBRIDGE1_STRUCT_fontations_ffi$ColorStop
struct ColorStop final {
  float stop;
  ::std::uint16_t palette_index;
  float alpha;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$ColorStop

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$Metrics
#define CXXBRIDGE1_STRUCT_fontations_ffi$Metrics
struct Metrics final {
  float top;
  float ascent;
  float descent;
  float bottom;
  float leading;
  float avg_char_width;
  float max_char_width;
  float x_min;
  float x_max;
  float x_height;
  float cap_height;
  float underline_position;
  float underline_thickness;
  float strikeout_position;
  float strikeout_thickness;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$Metrics

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedName
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedName
struct BridgeLocalizedName final {
  ::rust::String string;
  ::rust::String language;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedName

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$SkiaDesignCoordinate
#define CXXBRIDGE1_STRUCT_fontations_ffi$SkiaDesignCoordinate
struct SkiaDesignCoordinate final {
  ::std::uint32_t axis;
  float value;

  bool operator==(SkiaDesignCoordinate const &) const noexcept;
  bool operator!=(SkiaDesignCoordinate const &) const noexcept;
  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$SkiaDesignCoordinate

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeScalerMetrics
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeScalerMetrics
struct BridgeScalerMetrics final {
  bool has_overlaps;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeScalerMetrics

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$PaletteOverride
#define CXXBRIDGE1_STRUCT_fontations_ffi$PaletteOverride
struct PaletteOverride final {
  ::std::uint16_t index;
  ::std::uint32_t color_8888;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$PaletteOverride

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$ClipBox
#define CXXBRIDGE1_STRUCT_fontations_ffi$ClipBox
struct ClipBox final {
  float x_min;
  float y_min;
  float x_max;
  float y_max;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$ClipBox

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$Transform
#define CXXBRIDGE1_STRUCT_fontations_ffi$Transform
struct Transform final {
  float xx;
  float xy;
  float yx;
  float yy;
  float dx;
  float dy;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$Transform

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$FillLinearParams
#define CXXBRIDGE1_STRUCT_fontations_ffi$FillLinearParams
struct FillLinearParams final {
  float x0;
  float y0;
  float x1;
  float y1;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$FillLinearParams

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$FillRadialParams
#define CXXBRIDGE1_STRUCT_fontations_ffi$FillRadialParams
struct FillRadialParams final {
  float x0;
  float y0;
  float r0;
  float x1;
  float y1;
  float r1;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$FillRadialParams

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$FillSweepParams
#define CXXBRIDGE1_STRUCT_fontations_ffi$FillSweepParams
struct FillSweepParams final {
  float x0;
  float y0;
  float start_angle;
  float end_angle;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$FillSweepParams

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontStyle
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontStyle
struct BridgeFontStyle final {
  ::std::int32_t weight;
  ::std::int32_t slant;
  ::std::int32_t width;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontStyle

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BitmapMetrics
#define CXXBRIDGE1_STRUCT_fontations_ffi$BitmapMetrics
struct BitmapMetrics final {
  float bearing_x;
  float bearing_y;
  float ppem_x;
  float ppem_y;
  bool placement_origin_bottom_left;
  float inner_bearing_x;
  float inner_bearing_y;
  float advance;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BitmapMetrics

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontRef
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontRef
struct BridgeFontRef final : public ::rust::Opaque {
  ~BridgeFontRef() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeFontRef

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeOutlineCollection
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeOutlineCollection
struct BridgeOutlineCollection final : public ::rust::Opaque {
  ~BridgeOutlineCollection() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeOutlineCollection

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeMappingIndex
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeMappingIndex
struct BridgeMappingIndex final : public ::rust::Opaque {
  ~BridgeMappingIndex() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeMappingIndex

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeHintingInstance
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeHintingInstance
struct BridgeHintingInstance final : public ::rust::Opaque {
  ~BridgeHintingInstance() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeHintingInstance

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeBitmapGlyph
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeBitmapGlyph
struct BridgeBitmapGlyph final : public ::rust::Opaque {
  ~BridgeBitmapGlyph() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeBitmapGlyph

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedStrings
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedStrings
struct BridgeLocalizedStrings final : public ::rust::Opaque {
  ~BridgeLocalizedStrings() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeLocalizedStrings

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeNormalizedCoords
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeNormalizedCoords
struct BridgeNormalizedCoords final : public ::rust::Opaque {
  ~BridgeNormalizedCoords() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeNormalizedCoords

#ifndef CXXBRIDGE1_STRUCT_fontations_ffi$BridgeColorStops
#define CXXBRIDGE1_STRUCT_fontations_ffi$BridgeColorStops
struct BridgeColorStops final : public ::rust::Opaque {
  ~BridgeColorStops() = delete;

private:
  friend ::rust::layout;
  struct layout {
    static ::std::size_t size() noexcept;
    static ::std::size_t align() noexcept;
  };
};
#endif // CXXBRIDGE1_STRUCT_fontations_ffi$BridgeColorStops

::rust::Box<::fontations_ffi::BridgeFontRef> make_font_ref(::rust::Slice<::std::uint8_t const> font_data, ::std::uint32_t index) noexcept;

bool font_ref_is_valid(::fontations_ffi::BridgeFontRef const &bridge_font_ref) noexcept;

::rust::Box<::fontations_ffi::BridgeOutlineCollection> get_outline_collection(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

// Returns true on a font or collection, sets `num_fonts``
// to 0 if single font file, and to > 0 for a TrueType collection.
// Returns false if the data cannot be interpreted as a font or collection.
bool font_or_collection(::rust::Slice<::std::uint8_t const> font_data, ::std::uint32_t &num_fonts) noexcept;

::std::size_t num_named_instances(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

::rust::Box<::fontations_ffi::BridgeMappingIndex> make_mapping_index(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

::rust::Box<::fontations_ffi::BridgeHintingInstance> make_hinting_instance(::fontations_ffi::BridgeOutlineCollection const &outlines, float size, ::fontations_ffi::BridgeNormalizedCoords const &coords, bool do_lcd_antialiasing, bool lcd_orientation_vertical, bool preserve_linear_metrics) noexcept;

::rust::Box<::fontations_ffi::BridgeHintingInstance> make_mono_hinting_instance(::fontations_ffi::BridgeOutlineCollection const &outlines, float size, ::fontations_ffi::BridgeNormalizedCoords const &coords) noexcept;

::rust::Box<::fontations_ffi::BridgeHintingInstance> no_hinting_instance() noexcept;

::std::uint16_t lookup_glyph_or_zero(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::BridgeMappingIndex const &map, ::std::uint32_t codepoint) noexcept;

bool get_path(::fontations_ffi::BridgeOutlineCollection const &outlines, ::std::uint16_t glyph_id, float size, ::fontations_ffi::BridgeNormalizedCoords const &coords, ::fontations_ffi::BridgeHintingInstance const &hinting_instance, ::fontations_ffi::PathWrapper &path_wrapper, ::fontations_ffi::BridgeScalerMetrics &scaler_metrics) noexcept;

float unhinted_advance_width_or_zero(::fontations_ffi::BridgeFontRef const &font_ref, float size, ::fontations_ffi::BridgeNormalizedCoords const &coords, ::std::uint16_t glyph_id) noexcept;

bool scaler_hinted_advance_width(::fontations_ffi::BridgeOutlineCollection const &outlines, ::fontations_ffi::BridgeHintingInstance const &hinting_instance, ::std::uint16_t glyph_id, float &out_advance_width) noexcept;

::std::uint16_t units_per_em_or_zero(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

::fontations_ffi::Metrics get_skia_metrics(::fontations_ffi::BridgeFontRef const &font_ref, float size, ::fontations_ffi::BridgeNormalizedCoords const &coords) noexcept;

::fontations_ffi::Metrics get_unscaled_metrics(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::BridgeNormalizedCoords const &coords) noexcept;

::std::uint16_t num_glyphs(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

void fill_glyph_to_unicode_map(::fontations_ffi::BridgeFontRef const &font_ref, ::rust::Slice<::std::uint32_t > map) noexcept;

::rust::String family_name(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool postscript_name(::fontations_ffi::BridgeFontRef const &font_ref, ::rust::String &out_string) noexcept;

// Receives a slice of palette overrides that will be merged
// with the specified base palette of the font. The result is a
// palette of RGBA, 8-bit per component, colors, consisting of
// palette entries merged with overrides.
::rust::Vec<::std::uint32_t> resolve_palette(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint16_t base_palette, ::rust::Slice<::fontations_ffi::PaletteOverride const> palette_overrides) noexcept;

bool has_colrv1_glyph(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint16_t glyph_id) noexcept;

bool has_colrv0_glyph(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint16_t glyph_id) noexcept;

bool get_colrv1_clip_box(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::BridgeNormalizedCoords const &coords, ::std::uint16_t glyph_id, float size, ::fontations_ffi::ClipBox &clip_box) noexcept;

bool has_bitmap_glyph(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint16_t glyph_id) noexcept;

::rust::Box<::fontations_ffi::BridgeBitmapGlyph> bitmap_glyph(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint16_t glyph_id, float font_size) noexcept;

::rust::Slice<::std::uint8_t const> png_data(::fontations_ffi::BridgeBitmapGlyph const &bitmap_glyph) noexcept;

::fontations_ffi::BitmapMetrics const &bitmap_metrics(::fontations_ffi::BridgeBitmapGlyph const &bitmap_glyph) noexcept;

::std::size_t table_data(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint32_t tag, ::std::size_t offset, ::rust::Slice<::std::uint8_t > data) noexcept;

::std::uint16_t table_tags(::fontations_ffi::BridgeFontRef const &font_ref, ::rust::Slice<::std::uint32_t > tags) noexcept;

::rust::isize variation_position(::fontations_ffi::BridgeNormalizedCoords const &coords, ::rust::Slice<::fontations_ffi::SkiaDesignCoordinate > coordinates) noexcept;

::rust::isize coordinates_for_shifted_named_instance_index(::fontations_ffi::BridgeFontRef const &font_ref, ::std::uint32_t shifted_index, ::rust::Slice<::fontations_ffi::SkiaDesignCoordinate > coords) noexcept;

::std::size_t num_axes(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

::rust::isize populate_axes(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::AxisWrapper &axis_wrapper) noexcept;

::rust::Box<::fontations_ffi::BridgeLocalizedStrings> get_localized_strings(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool localized_name_next(::fontations_ffi::BridgeLocalizedStrings &bridge_localized_strings, ::fontations_ffi::BridgeLocalizedName &out_localized_name) noexcept;

::rust::Box<::fontations_ffi::BridgeNormalizedCoords> resolve_into_normalized_coords(::fontations_ffi::BridgeFontRef const &font_ref, ::rust::Slice<::fontations_ffi::SkiaDesignCoordinate const> design_coords) noexcept;

bool normalized_coords_equal(::fontations_ffi::BridgeNormalizedCoords const &a, ::fontations_ffi::BridgeNormalizedCoords const &b) noexcept;

bool draw_colr_glyph(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::BridgeNormalizedCoords const &coords, ::std::uint16_t glyph_id, ::fontations_ffi::ColorPainterWrapper &color_painter) noexcept;

bool next_color_stop(::fontations_ffi::BridgeColorStops &color_stops, ::fontations_ffi::ColorStop &stop) noexcept;

::std::size_t num_color_stops(::fontations_ffi::BridgeColorStops const &color_stops) noexcept;

bool get_font_style(::fontations_ffi::BridgeFontRef const &font_ref, ::fontations_ffi::BridgeNormalizedCoords const &coords, ::fontations_ffi::BridgeFontStyle &font_style) noexcept;

bool is_embeddable(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool is_subsettable(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool is_fixed_pitch(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool is_serif_style(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

bool is_script_style(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;

::std::int32_t italic_angle(::fontations_ffi::BridgeFontRef const &font_ref) noexcept;
} // namespace fontations_ffi
