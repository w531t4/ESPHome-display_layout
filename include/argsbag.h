// argsbag.h -- from chatgpt
// -----------------------------------------------------------------------------
// ArgsBag: A lightweight, RTTI-free heterogeneous key–value store.
//
// PURPOSE
//   - Carry multiple, differently-typed “extras” through your InitArgs/PostArgs,
//     without using std::any / std::type_index / typeid (compatible with -fno-rtti).
//
// KEY IDEAS
//   - Each stored type T gets a unique key using the address of a TU-local static
//     (see type_key<T>()). No RTTI is required.
//   - For each type T, at most ONE value is stored at a time. A later set<T>()
//     replaces the earlier one.
//   - Values are heap-allocated and owned by ArgsBag; copy/move constructors
//     perform deep copies / ownership transfer.
//
// COMPLEXITY
//   - Operations are average O(1) via std::unordered_map.
//
// LIFETIME / OWNERSHIP
//   - ArgsBag owns stored objects. clear(), erase<T>(), destructor release them.
//   - Copying ArgsBag deep-copies each stored object via a per-type cloner.
//   - Moving ArgsBag transfers ownership and clears the moved-from bag.
//
// THREAD-SAFETY
//   - Not thread-safe. Synchronize externally if needed.
//
// LIMITATIONS
//   - One value per type T. If you need multiple values of the same T, change the
//     storage to std::vector<Entry> per key (see notes in README / comments).
//   - Uses operator new/delete; with -fno-exceptions, allocation failures will
//     call std::terminate by default (as usual). No exceptions are thrown here.
//
// EXAMPLE
//   struct TextInitArgs { std::optional<bool> right_align; /* ... */ };
//   struct Colors { esphome::Color user, msg; };
//
//   InitArgs ia;
//   ia.extras = ArgsBag::of(TextInitArgs{.right_align=true},
//                           Colors{.user=YELLOW, .msg=WHITE});
//
//   if (auto* t = ia.extras.get<TextInitArgs>()) { /* use *t */ }
//   if (ia.extras.has<Colors>()) { /* ... */ }
//
// -----------------------------------------------------------------------------
#pragma once
#include <unordered_map>
#include <utility>
#include <cstddef>

class ArgsBag {
    /// Internal storage entry with type-erased pointer and ops.
    struct Entry {
        void* ptr = nullptr;                      ///< Owned pointer to T
        void (*deleter)(void*) = nullptr;         ///< Deletes a T*
        void* (*cloner)(const void*) = nullptr;   ///< Returns new T* cloned from existing
    };

    /// Unique key per T without RTTI: address of a TU-local static.
    /// Safe across a single binary (embedded/monolith). If you cross DSO boundaries,
    /// ensure each T is defined in the same binary to preserve uniqueness.
    template <class T>
    static const void* type_key() noexcept {
        static int unique;
        return &unique;
    }

public:
    // -----------------------
    // Special member functions
    // -----------------------

    /// Default construct: empty bag.
    ArgsBag() = default;

    /// Deep-copy constructor: clones each stored value.
    ArgsBag(const ArgsBag& other) {
        for (const auto& [k, e] : other.items_) {
            items_[k] = Entry{ e.cloner ? e.cloner(e.ptr) : nullptr,
                               e.deleter, e.cloner };
        }
    }

    /// Deep-copy assignment: clears current, then clones from other.
    ArgsBag& operator=(const ArgsBag& other) {
        if (this == &other) return *this;
        clear();
        for (const auto& [k, e] : other.items_) {
            items_[k] = Entry{ e.cloner ? e.cloner(e.ptr) : nullptr,
                               e.deleter, e.cloner };
        }
        return *this;
    }

    /// Move constructor: transfers ownership; leaves other empty.
    ArgsBag(ArgsBag&& other) noexcept : items_(std::move(other.items_)) {
        other.items_.clear();
    }

    /// Move assignment: releases current, takes other's storage; leaves other empty.
    ArgsBag& operator=(ArgsBag&& other) noexcept {
        if (this != &other) {
            clear();
            items_ = std::move(other.items_);
            other.items_.clear();
        }
        return *this;
    }

    /// Destructor: releases all stored objects.
    ~ArgsBag() { clear(); }

    // -------------
    // Modifiers (set / emplace / erase / clear)
    // -------------

    /**
     * @brief Store (or replace) a value of type T.
     *
     * If a value of type T already exists, it is destroyed and replaced.
     * @tparam T  The concrete type being stored.
     * @param value  Value to store (moved into the bag).
     */
    template <class T>
    void set(T value) {
        const void* key = type_key<T>();
        // destroy old if present
        auto it = items_.find(key);
        if (it != items_.end()) {
            if (it->second.deleter) it->second.deleter(it->second.ptr);
        }
        // allocate & store new
        T* p = new T(std::move(value));
        items_[key] = Entry{
            p,
            [](void* q){ delete static_cast<T*>(q); },
            [](const void* q)->void* { return new T(*static_cast<const T*>(q)); }
        };
    }

    /**
     * @brief Construct a value of type T in-place and store it.
     *
     * Replaces any existing T. Returns a reference to the stored T.
     */
    template <class T, class... A>
    T& emplace(A&&... a) {
        const void* key = type_key<T>();
        auto it = items_.find(key);
        if (it != items_.end()) {
            if (it->second.deleter) it->second.deleter(it->second.ptr);
        }
        T* p = new T(std::forward<A>(a)...);
        items_[key] = Entry{
            p,
            [](void* q){ delete static_cast<T*>(q); },
            [](const void* q)->void* { return new T(*static_cast<const T*>(q)); }
        };
        return *p;
    }

    /**
     * @brief Remove a stored value of type T, if present.
     * @return true if a value of type T was found and erased; false otherwise.
     */
    template <class T>
    bool erase() {
        auto it = items_.find(type_key<T>());
        if (it == items_.end()) return false;
        if (it->second.deleter) it->second.deleter(it->second.ptr);
        items_.erase(it);
        return true;
    }

    /// Remove all stored values and free their memory.
    void clear() noexcept {
        for (auto& [k, e] : items_) if (e.deleter) e.deleter(e.ptr);
        items_.clear();
    }

    // -------------
    // Lookup (get / has)
    // -------------

    /**
     * @brief Get a mutable pointer to the stored T, or nullptr if absent.
     * @return T* or nullptr.
     */
    template <class T>
    T* get() noexcept {
        auto it = items_.find(type_key<T>());
        return (it == items_.end()) ? nullptr : static_cast<T*>(it->second.ptr);
    }

    /**
     * @brief Get a const pointer to the stored T, or nullptr if absent.
     * @return const T* or nullptr.
     */
    template <class T>
    const T* get() const noexcept {
        auto it = items_.find(type_key<T>());
        return (it == items_.end()) ? nullptr : static_cast<const T*>(it->second.ptr);
    }

    /**
     * @brief Check if a value of type T is present.
     * @return true if present; false otherwise.
     */
    template <class T>
    bool has() const noexcept {
        return items_.find(type_key<T>()) != items_.end();
    }

    // -------------
    // Convenience factory
    // -------------

    /**
     * @brief Build an ArgsBag from multiple values in one call.
     *
     * Usage:
     *   auto bag = ArgsBag::of(TextInitArgs{...}, Colors{...}, ...);
     */
    template<class... Ts>
    static ArgsBag of(Ts&&... xs) {
        ArgsBag b;
        (b.set(std::forward<Ts>(xs)), ...);   // call set(...) for each arg
        return b;
    }

private:
    std::unordered_map<const void*, Entry> items_;  ///< map: type key -> entry
};
