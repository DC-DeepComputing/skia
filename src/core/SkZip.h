/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkZip_DEFINED
#define SkZip_DEFINED

#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"

// Create a tuple like class here, so that an rvalue swap can be added and found through ADL.
// This means adding back get, tuple_size, and tuple_element.
template<typename... Ts>
class SkZipReturnTuple : public std::tuple<Ts&...>{
public:
    using std::tuple<Ts&...>::tuple;
    using std::tuple<Ts&...>::operator=;
    using INHERITED = std::tuple<Ts&...>;

    template<std::size_t I>
    auto&& get()       &  { return std::get<I>(static_cast<INHERITED&>(*this)); }

    template<std::size_t I>
    auto&& get()       && { return std::get<I>(static_cast<INHERITED&&>(*this)); }

    template<std::size_t I>
    auto&& get() const &  { return std::get<I>(static_cast<const INHERITED&>(*this)); }

    template<std::size_t I>
    auto&& get() const && { return std::get<I>(static_cast<const INHERITED&&>(*this)); }

    friend void swap(SkZipReturnTuple& a, SkZipReturnTuple& b) {
        using std::swap;
        swap(static_cast<INHERITED&>(a), static_cast<INHERITED&>(b));
    }
    friend void swap(SkZipReturnTuple&& a, SkZipReturnTuple&& b) {
        using std::swap;
        // This entire exercise was to make these rvalues into lvalues here.
        swap(static_cast<INHERITED&>(a), static_cast<INHERITED&>(b));
    }
};

namespace std {
  // Template arguments of an outer class cannot be deduced, so SkZipReturnTuple must be top level.
  template<typename... Ts> struct tuple_size<typename ::SkZipReturnTuple<Ts...>>
      : public tuple_size<typename ::SkZipReturnTuple<Ts...>::INHERITED> {};
  template<std::size_t I, typename... Ts> struct tuple_element<I, typename ::SkZipReturnTuple<Ts...>>
      : public tuple_element<I, typename ::SkZipReturnTuple<Ts...>::INHERITED>{};
}

// Take a list of things that can be pointers, and use them all in parallel. The iterators and
// accessor operator[] for the class produce a tuple of the items.
template<typename... Ts>
class SkZip {
    using ReturnTuple = SkZipReturnTuple<Ts...>;

    template<bool C> class Iterator {
    public:
        using value_type = ReturnTuple;
        using difference_type = ptrdiff_t;
        using pointer = typename std::conditional_t<C, value_type const*, value_type*>;
        using reference = typename std::conditional_t<C, value_type const, value_type>;
        using iterator_category = std::random_access_iterator_tag;
        constexpr Iterator(const SkZip* zip, size_t index) : fZip{zip}, fIndex{index} { }
        constexpr Iterator(const Iterator&) = default;
        constexpr Iterator& operator=(const Iterator& that) {
            SkASSERT(this->fZip == that.fZip); this->fIndex = that.fIndex; return *this;
        }
        constexpr Iterator& operator++() { ++fIndex; return *this; }
        constexpr Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        constexpr Iterator& operator+=(difference_type n) { fIndex += n; return *this; }
        friend constexpr Iterator operator+(Iterator lhs, difference_type n) { return lhs += n; }
        friend constexpr Iterator operator+(difference_type n, Iterator rhs) { return rhs += n; }
        constexpr Iterator& operator--() { --fIndex; return *this; }
        constexpr Iterator operator--(int) { Iterator tmp(*this); operator--(); return tmp; }
        constexpr Iterator& operator-=(difference_type n) { fIndex -= n; return *this; }
        friend constexpr Iterator operator-(Iterator lhs, difference_type n) { return lhs -= n; }
        constexpr bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        constexpr bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        constexpr bool operator<(const Iterator rhs) const { return fIndex < rhs.fIndex; }
        constexpr bool operator<=(const Iterator rhs) const { return fIndex <= rhs.fIndex; }
        constexpr bool operator>(const Iterator rhs) const { return fIndex > rhs.fIndex; }
        constexpr bool operator>=(const Iterator rhs) const { return fIndex >= rhs.fIndex; }
        constexpr reference operator*() const { return (*fZip)[fIndex]; }
        constexpr reference operator[](difference_type i) const { return (*fZip)[fIndex + i]; }
        friend constexpr difference_type operator-(Iterator lhs, Iterator rhs) {
            return lhs.fIndex - rhs.fIndex;
        }

    private:
        SkZip const * const fZip = nullptr;
        size_t fIndex = 0;
    };

    template<typename T>
    inline static constexpr T* nullify = nullptr;

public:
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    constexpr SkZip() : fPointers{nullify<Ts>...}, fSize{0} {}
    constexpr SkZip(size_t) = delete;
    constexpr SkZip(size_t size, Ts*... ts)
            : fPointers{ts...}
            , fSize{size} {}
    constexpr SkZip(const SkZip& that) = default;
    constexpr SkZip& operator=(const SkZip &that) = default;

    // Check to see if U can be used for const T or is the same as T
    template <typename U, typename T>
    using CanConvertToConst = typename std::integral_constant<bool,
                    std::is_convertible<U*, T*>::value && sizeof(U) == sizeof(T)>::type;

    // Allow SkZip<const T> to be constructed from SkZip<T>.
    template<typename... Us,
            typename = std::enable_if<std::conjunction<CanConvertToConst<Us, Ts>...>::value>>
    constexpr SkZip(const SkZip<Us...>& that)
        : fPointers(that.data())
        , fSize{that.size()} { }

    constexpr ReturnTuple operator[](size_t i) const { return this->index(i);}
    constexpr size_t size() const { return fSize; }
    constexpr bool empty() const { return this->size() == 0; }
    constexpr ReturnTuple front() const { return this->index(0); }
    constexpr ReturnTuple back() const { return this->index(this->size() - 1); }
    /** begin and end are marked const to allow std::begin and std::end to work on SkZip xvalues. */
    constexpr iterator begin() const { return iterator{this, 0}; }
    constexpr iterator end() const { return iterator{this, this->size()}; }
    constexpr const_iterator cbegin() const { return const_iterator{this, 0}; }
    constexpr const_iterator cend() const { return const_iterator{this, this->size()}; }
    template<size_t I> constexpr auto get() const {
        return SkMakeSpan(std::get<I>(fPointers), fSize);
    }
    constexpr std::tuple<Ts*...> data() const { return fPointers; }
    constexpr SkZip first(size_t n) const {
        SkASSERT(n <= this->size());
        if (n == 0) { return SkZip(); }
        return SkZip{n, fPointers};
    }
    constexpr SkZip last(size_t n) const {
        SkASSERT(n <= this->size());
        if (n == 0) { return SkZip(); }
        return SkZip{n, this->pointersAt(fSize - n)};
    }
    constexpr SkZip subspan(size_t offset, size_t count) const {
        SkASSERT(offset < this->size());
        SkASSERT(count <= this->size() - offset);
        if (count == 0) { return SkZip(); }
        return SkZip(count, pointersAt(offset));
    }

private:
    constexpr SkZip(size_t n, const std::tuple<Ts*...>& pointers)
        : fPointers{pointers}
        , fSize{n} {}

    constexpr ReturnTuple index(size_t i) const {
        SkASSERT(this->size() > 0);
        SkASSERT(i < this->size());
        return indexDetail(i, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    constexpr ReturnTuple indexDetail(size_t i, std::index_sequence<Is...>) const {
        return ReturnTuple((std::get<Is>(fPointers))[i]...);
    }

    std::tuple<Ts*...> pointersAt(size_t i) const {
        SkASSERT(this->size() > 0);
        SkASSERT(i < this->size());
        return pointersAtDetail(i, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    constexpr std::tuple<Ts*...> pointersAtDetail(size_t i, std::index_sequence<Is...>) const {
        return std::tuple<Ts*...>{&(std::get<Is>(fPointers))[i]...};
    }

    std::tuple<Ts*...> fPointers;
    size_t fSize;
};

class SkMakeZipDetail {
    template<typename T> struct DecayPointer{
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        using type = typename std::conditional<std::is_pointer<U>::value, U, T>::type;
    };
    template<typename T> using DecayPointerT = typename DecayPointer<T>::type;

    template<typename C> struct ContiguousMemory { };
    template<typename T> struct ContiguousMemory<T*> {
        using value_type = T;
        static constexpr value_type* Data(T* t) { return t; }
        static constexpr size_t Size(T* s) { return SIZE_MAX; }
    };
    template<typename T, size_t N> struct ContiguousMemory<T(&)[N]> {
        using value_type = T;
        static constexpr value_type* Data(T(&t)[N]) { return t; }
        static constexpr size_t Size(T(&)[N]) { return N; }
    };
    // In general, we don't want r-value collections, but SkSpans are ok, because they are a view
    // onto an actual container.
    template<typename T> struct ContiguousMemory<SkSpan<T>> {
        using value_type = T;
        static constexpr value_type* Data(SkSpan<T> s) { return s.data(); }
        static constexpr size_t Size(SkSpan<T> s) { return s.size(); }
    };
    // Only accept l-value references to collections.
    template<typename C> struct ContiguousMemory<C&> {
        using value_type = typename std::remove_pointer<decltype(std::declval<C>().data())>::type;
        static constexpr value_type* Data(C& c) { return c.data(); }
        static constexpr size_t Size(C& c) { return c.size(); }
    };
    template<typename C> using Span = ContiguousMemory<DecayPointerT<C>>;
    template<typename C> using ValueType = typename Span<C>::value_type;

    template<typename C, typename... Ts> struct PickOneSize { };
    template <typename T, typename... Ts> struct PickOneSize<T*, Ts...> {
        static constexpr size_t Size(T* t, Ts... ts) {
            return PickOneSize<Ts...>::Size(std::forward<Ts>(ts)...);
        }
    };
    template <typename T, typename... Ts, size_t N> struct PickOneSize<T(&)[N], Ts...> {
        static constexpr size_t Size(T(&)[N], Ts...) { return N; }
    };
    template<typename T, typename... Ts> struct PickOneSize<SkSpan<T>, Ts...> {
        static constexpr size_t Size(SkSpan<T> s, Ts...) { return s.size(); }
    };
    template<typename C, typename... Ts> struct PickOneSize<C&, Ts...> {
        static constexpr size_t Size(C& c, Ts...) { return c.size(); }
    };

public:
    template<typename... Ts>
    static constexpr auto MakeZip(Ts&& ... ts) {

        // Pick the first collection that has a size, and use that for the size.
        size_t size = PickOneSize<DecayPointerT<Ts>...>::Size(std::forward<Ts>(ts)...);

#ifdef SK_DEBUG
        // Check that all sizes are the same.
        size_t minSize = SIZE_MAX;
        size_t maxSize = 0;
        for (size_t s : {Span<Ts>::Size(std::forward<Ts>(ts))...}) {
            if (s != SIZE_MAX) {
                minSize = std::min(minSize, s);
                maxSize = std::max(maxSize, s);
            }
        }
        SkASSERT(minSize == maxSize);
#endif

        return SkZip<ValueType<Ts>...>{size, Span<Ts>::Data(std::forward<Ts>(ts))...};
    }
};

template<typename... Ts>
inline constexpr auto SkMakeZip(Ts&& ... ts) {
    return SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...);
}
#endif //SkZip_DEFINED
