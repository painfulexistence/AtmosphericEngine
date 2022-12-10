#include <memory>
#include <utility>
#include <iterator>
#include <exception>
#include <type_traits>
#include <unordered_set>
#include <gtest/gtest.h>
#include <entt/entity/component.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/storage.hpp>
#include "throwing_allocator.hpp"
#include "throwing_component.hpp"

struct empty_type {};
struct boxed_int { int value; };
struct stable_type { int value; };

struct update_from_destructor {
    update_from_destructor(entt::storage<update_from_destructor> &ref, entt::entity other)
        : storage{&ref},
          target{other}
    {}

    update_from_destructor(update_from_destructor &&other) ENTT_NOEXCEPT
        : storage{std::exchange(other.storage, nullptr)},
          target{std::exchange(other.target, entt::null)}
    {}

    update_from_destructor & operator=(update_from_destructor &&other) ENTT_NOEXCEPT {
        storage = std::exchange(other.storage, nullptr);
        target = std::exchange(other.target, entt::null);
        return *this;
    }

    ~update_from_destructor() {
        if(target != entt::null && storage->contains(target)) {
            storage->erase(target);
        }
    }

private:
    entt::storage<update_from_destructor> *storage{};
    entt::entity target{entt::null};
};

template<>
struct entt::component_traits<stable_type>: basic_component_traits {
    using in_place_delete = std::true_type;
};

bool operator==(const boxed_int &lhs, const boxed_int &rhs) {
    return lhs.value == rhs.value;
}

TEST(Storage, Functionalities) {
    entt::storage<int> pool;

    pool.reserve(42);

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);
    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0u);
    ASSERT_EQ(std::as_const(pool).begin(), std::as_const(pool).end());
    ASSERT_EQ(pool.begin(), pool.end());
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_FALSE(pool.contains(entt::entity{41}));

    pool.reserve(0);

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);
    ASSERT_TRUE(pool.empty());

    pool.emplace(entt::entity{41}, 3);

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 1u);
    ASSERT_NE(std::as_const(pool).begin(), std::as_const(pool).end());
    ASSERT_NE(pool.begin(), pool.end());
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_TRUE(pool.contains(entt::entity{41}));
    ASSERT_EQ(pool.get(entt::entity{41}), 3);

    pool.erase(entt::entity{41});

    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0u);
    ASSERT_EQ(std::as_const(pool).begin(), std::as_const(pool).end());
    ASSERT_EQ(pool.begin(), pool.end());
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_FALSE(pool.contains(entt::entity{41}));

    pool.emplace(entt::entity{41}, 12);

    ASSERT_EQ(pool.get(entt::entity{41}), 12);

    pool.clear();

    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0u);
    ASSERT_EQ(std::as_const(pool).begin(), std::as_const(pool).end());
    ASSERT_EQ(pool.begin(), pool.end());
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_FALSE(pool.contains(entt::entity{41}));

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);

    pool.shrink_to_fit();

    ASSERT_EQ(pool.capacity(), 0u);
}

TEST(Storage, Move) {
    entt::storage<int> pool;
    pool.emplace(entt::entity{3}, 3);

    ASSERT_TRUE(std::is_move_constructible_v<decltype(pool)>);
    ASSERT_TRUE(std::is_move_assignable_v<decltype(pool)>);

    entt::storage<int> other{std::move(pool)};

    ASSERT_TRUE(pool.empty());
    ASSERT_FALSE(other.empty());
    ASSERT_EQ(pool.at(0u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(other.at(0u), entt::entity{3});
    ASSERT_EQ(other.get(entt::entity{3}), 3);

    pool = std::move(other);

    ASSERT_FALSE(pool.empty());
    ASSERT_TRUE(other.empty());
    ASSERT_EQ(pool.at(0u), entt::entity{3});
    ASSERT_EQ(pool.get(entt::entity{3}), 3);
    ASSERT_EQ(other.at(0u), static_cast<entt::entity>(entt::null));

    other = entt::storage<int>{};
    other.emplace(entt::entity{42}, 42);
    other = std::move(pool);

    ASSERT_TRUE(pool.empty());
    ASSERT_FALSE(other.empty());
    ASSERT_EQ(pool.at(0u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(other.at(0u), entt::entity{3});
    ASSERT_EQ(other.get(entt::entity{3}), 3);
}

TEST(Storage, EmptyType) {
    entt::storage<empty_type> pool;
    pool.emplace(entt::entity{99});

    ASSERT_TRUE(pool.contains(entt::entity{99}));
}

TEST(Storage, Insert) {
    entt::storage<stable_type> pool;
    entt::entity entities[2u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    pool.insert(std::begin(entities), std::end(entities), stable_type{99});

    ASSERT_TRUE(pool.contains(entities[0u]));
    ASSERT_TRUE(pool.contains(entities[1u]));

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 2u);
    ASSERT_EQ(pool.get(entities[0u]).value, 99);
    ASSERT_EQ(pool.get(entities[1u]).value, 99);

    pool.erase(std::begin(entities), std::end(entities));
    const stable_type values[2u] = { stable_type{42}, stable_type{3} };
    pool.insert(std::rbegin(entities), std::rend(entities), std::begin(values));

    ASSERT_EQ(pool.size(), 4u);
    ASSERT_TRUE(pool.at(0u) == entt::tombstone);
    ASSERT_TRUE(pool.at(1u) == entt::tombstone);
    ASSERT_EQ(pool.at(2u), entities[1u]);
    ASSERT_EQ(pool.at(3u), entities[0u]);
    ASSERT_EQ(pool.index(entities[0u]), 3u);
    ASSERT_EQ(pool.index(entities[1u]), 2u);
    ASSERT_EQ(pool.get(entities[0u]).value, 3);
    ASSERT_EQ(pool.get(entities[1u]).value, 42);
}

TEST(Storage, InsertEmptyType) {
    entt::storage<empty_type> pool;
    entt::entity entities[2u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};

    pool.insert(std::begin(entities), std::end(entities));

    ASSERT_TRUE(pool.contains(entities[0u]));
    ASSERT_TRUE(pool.contains(entities[1u]));

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 2u);
}

TEST(Storage, Erase) {
    entt::storage<int> pool;
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    pool.emplace(entities[0u]);
    pool.emplace(entities[1u]);
    pool.emplace(entities[2u]);
    pool.erase(std::begin(entities), std::end(entities));

    ASSERT_DEATH(pool.erase(std::begin(entities), std::end(entities)), "");
    ASSERT_TRUE(pool.empty());

    pool.emplace(entities[0u], 0);
    pool.emplace(entities[1u], 1);
    pool.emplace(entities[2u], 2);
    pool.erase(entities, entities + 2u);

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(*pool.begin(), 2);

    pool.erase(entities[2u]);

    ASSERT_DEATH(pool.erase(entities[2u]), "");
    ASSERT_TRUE(pool.empty());

    pool.emplace(entities[0u], 0);
    pool.emplace(entities[1u], 1);
    pool.emplace(entities[2u], 2);
    std::swap(entities[1u], entities[2u]);
    pool.erase(entities, entities + 2u);

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(*pool.begin(), 1);
}

TEST(Storage, StableErase) {
    entt::storage<stable_type> pool;
    entt::entity entities[3u];

    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::tombstone), "");
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::null), "");

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});
    pool.emplace(entities[2u], stable_type{2});

    pool.erase(std::begin(entities), std::end(entities));

    ASSERT_DEATH(pool.erase(std::begin(entities), std::end(entities)), "");
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.at(2u) == entt::tombstone);
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::tombstone), "");
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::null), "");
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entities[1u]), "");

    pool.emplace(entities[2u], stable_type{2});
    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});

    ASSERT_EQ(pool.get(entities[0u]).value, 0);
    ASSERT_EQ(pool.get(entities[1u]).value, 1);
    ASSERT_EQ(pool.get(entities[2u]).value, 2);

    ASSERT_EQ(pool.begin()->value, 2);
    ASSERT_EQ(pool.index(entities[0u]), 1u);
    ASSERT_EQ(pool.index(entities[1u]), 0u);
    ASSERT_EQ(pool.index(entities[2u]), 2u);

    pool.erase(entities, entities + 2u);

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_EQ(pool.begin()->value, 2);
    ASSERT_EQ(pool.index(entities[2u]), 2u);

    pool.erase(entities[2u]);

    ASSERT_DEATH(pool.erase(entities[2u]), "");
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_FALSE(pool.contains(entities[0u]));
    ASSERT_FALSE(pool.contains(entities[1u]));
    ASSERT_FALSE(pool.contains(entities[2u]));

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});
    pool.emplace(entities[2u], stable_type{2});
    std::swap(entities[1u], entities[2u]);
    pool.erase(entities, entities + 2u);

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.contains(entities[2u]));
    ASSERT_EQ(pool.index(entities[2u]), 0u);
    ASSERT_EQ(pool.get(entities[2u]).value, 1);

    pool.compact();

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 1u);
    ASSERT_EQ(pool.begin()->value, 1);

    pool.clear();

    ASSERT_EQ(pool.size(), 0u);

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{2});
    pool.emplace(entities[2u], stable_type{1});
    pool.erase(entities[2u]);

    ASSERT_DEATH(pool.erase(entities[2u]), "");

    pool.erase(entities[0u]);
    pool.erase(entities[1u]);

    ASSERT_DEATH(pool.erase(entities, entities + 2u), "");
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.at(2u) == entt::tombstone);

    pool.emplace(entities[0u], stable_type{99});

    ASSERT_EQ((++pool.begin())->value, 99);

    pool.emplace(entities[1u], stable_type{2});
    pool.emplace(entities[2u], stable_type{1});
    pool.emplace(entt::entity{0}, stable_type{7});

    ASSERT_EQ(pool.size(), 4u);
    ASSERT_EQ(pool.begin()->value, 7);
    ASSERT_EQ(pool.at(0u), entities[1u]);
    ASSERT_EQ(pool.at(1u), entities[0u]);
    ASSERT_EQ(pool.at(2u), entities[2u]);

    ASSERT_EQ(pool.get(entities[0u]).value, 99);
    ASSERT_EQ(pool.get(entities[1u]).value, 2);
    ASSERT_EQ(pool.get(entities[2u]).value, 1);
}

TEST(Storage, Remove) {
    entt::storage<int> pool;
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    pool.emplace(entities[0u]);
    pool.emplace(entities[1u]);
    pool.emplace(entities[2u]);

    ASSERT_EQ(pool.remove(std::begin(entities), std::end(entities)), 3u);
    ASSERT_EQ(pool.remove(std::begin(entities), std::end(entities)), 0u);
    ASSERT_TRUE(pool.empty());

    pool.emplace(entities[0u], 0);
    pool.emplace(entities[1u], 1);
    pool.emplace(entities[2u], 2);

    ASSERT_EQ(pool.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(*pool.begin(), 2);

    ASSERT_EQ(pool.remove(entities[2u]), 1u);
    ASSERT_EQ(pool.remove(entities[2u]), 0u);
    ASSERT_TRUE(pool.empty());

    pool.emplace(entities[0u], 0);
    pool.emplace(entities[1u], 1);
    pool.emplace(entities[2u], 2);
    std::swap(entities[1u], entities[2u]);

    ASSERT_EQ(pool.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(*pool.begin(), 1);
}

TEST(Storage, StableRemove) {
    entt::storage<stable_type> pool;
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});
    pool.emplace(entities[2u], stable_type{2});

    ASSERT_EQ(pool.remove(std::begin(entities), std::end(entities)), 3u);
    ASSERT_EQ(pool.remove(std::begin(entities), std::end(entities)), 0u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.at(2u) == entt::tombstone);
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::tombstone), "");
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entt::null), "");
    ASSERT_DEATH([[maybe_unused]] auto &&value = pool.get(entities[1u]), "");

    pool.emplace(entities[2u], stable_type{2});
    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});

    ASSERT_EQ(pool.get(entities[0u]).value, 0);
    ASSERT_EQ(pool.get(entities[1u]).value, 1);
    ASSERT_EQ(pool.get(entities[2u]).value, 2);

    ASSERT_EQ(pool.begin()->value, 2);
    ASSERT_EQ(pool.index(entities[0u]), 1u);
    ASSERT_EQ(pool.index(entities[1u]), 0u);
    ASSERT_EQ(pool.index(entities[2u]), 2u);

    ASSERT_EQ(pool.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_EQ(pool.begin()->value, 2);
    ASSERT_EQ(pool.index(entities[2u]), 2u);

    ASSERT_EQ(pool.remove(entities[2u]), 1u);
    ASSERT_EQ(pool.remove(entities[2u]), 0u);
    ASSERT_EQ(pool.remove(entities[2u]), 0u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_FALSE(pool.contains(entities[0u]));
    ASSERT_FALSE(pool.contains(entities[1u]));
    ASSERT_FALSE(pool.contains(entities[2u]));

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{1});
    pool.emplace(entities[2u], stable_type{2});
    std::swap(entities[1u], entities[2u]);

    ASSERT_EQ(pool.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.contains(entities[2u]));
    ASSERT_EQ(pool.index(entities[2u]), 0u);
    ASSERT_EQ(pool.get(entities[2u]).value, 1);

    pool.compact();

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 1u);
    ASSERT_EQ(pool.begin()->value, 1);

    pool.clear();

    ASSERT_EQ(pool.size(), 0u);

    pool.emplace(entities[0u], stable_type{0});
    pool.emplace(entities[1u], stable_type{2});
    pool.emplace(entities[2u], stable_type{1});

    ASSERT_EQ(pool.remove(entities[2u]), 1u);
    ASSERT_EQ(pool.remove(entities[2u]), 0u);

    ASSERT_EQ(pool.remove(entities[0u]), 1u);
    ASSERT_EQ(pool.remove(entities[1u]), 1u);
    ASSERT_EQ(pool.remove(entities, entities + 2u), 0u);

    ASSERT_EQ(pool.size(), 3u);
    ASSERT_TRUE(pool.at(2u) == entt::tombstone);

    pool.emplace(entities[0u], stable_type{99});

    ASSERT_EQ((++pool.begin())->value, 99);

    pool.emplace(entities[1u], stable_type{2});
    pool.emplace(entities[2u], stable_type{1});
    pool.emplace(entt::entity{0}, stable_type{7});

    ASSERT_EQ(pool.size(), 4u);
    ASSERT_EQ(pool.begin()->value, 7);
    ASSERT_EQ(pool.at(0u), entities[1u]);
    ASSERT_EQ(pool.at(1u), entities[0u]);
    ASSERT_EQ(pool.at(2u), entities[2u]);

    ASSERT_EQ(pool.get(entities[0u]).value, 99);
    ASSERT_EQ(pool.get(entities[1u]).value, 2);
    ASSERT_EQ(pool.get(entities[2u]).value, 1);
}

TEST(Storage, Compact) {
    entt::storage<stable_type> pool;

    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0u);

    pool.compact();

    ASSERT_TRUE(pool.empty());
    ASSERT_EQ(pool.size(), 0u);

    pool.emplace(entt::entity{0}, stable_type{0});
    pool.compact();

    ASSERT_FALSE(pool.empty());
    ASSERT_EQ(pool.size(), 1u);

    pool.emplace(entt::entity{42}, stable_type{42});
    pool.erase(entt::entity{0});

    ASSERT_EQ(pool.size(), 2u);
    ASSERT_EQ(pool.index(entt::entity{42}), 1u);
    ASSERT_EQ(pool.get(entt::entity{42}).value, 42);

    pool.compact();

    ASSERT_EQ(pool.size(), 1u);
    ASSERT_EQ(pool.index(entt::entity{42}), 0u);
    ASSERT_EQ(pool.get(entt::entity{42}).value, 42);

    pool.emplace(entt::entity{0}, stable_type{0});
    pool.compact();

    ASSERT_EQ(pool.size(), 2u);
    ASSERT_EQ(pool.index(entt::entity{42}), 0u);
    ASSERT_EQ(pool.index(entt::entity{0}), 1u);
    ASSERT_EQ(pool.get(entt::entity{42}).value, 42);
    ASSERT_EQ(pool.get(entt::entity{0}).value, 0);

    pool.erase(entt::entity{0});
    pool.erase(entt::entity{42});
    pool.compact();

    ASSERT_TRUE(pool.empty());
}

TEST(Storage, ShrinkToFit) {
    entt::storage<int> pool;

    for(std::size_t next{}; next < ENTT_PACKED_PAGE; ++next) {
        pool.emplace(entt::entity(next));
    }

    pool.emplace(entt::entity{ENTT_PACKED_PAGE});
    pool.erase(entt::entity{ENTT_PACKED_PAGE});

    ASSERT_EQ(pool.capacity(), 2 * ENTT_PACKED_PAGE);
    ASSERT_EQ(pool.size(), ENTT_PACKED_PAGE);

    pool.shrink_to_fit();

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);
    ASSERT_EQ(pool.size(), ENTT_PACKED_PAGE);

    pool.clear();

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);
    ASSERT_EQ(pool.size(), 0u);

    pool.shrink_to_fit();

    ASSERT_EQ(pool.capacity(), 0u);
    ASSERT_EQ(pool.size(), 0u);
}

TEST(Storage, AggregatesMustWork) {
    struct aggregate_type { int value; };
    // the goal of this test is to enforce the requirements for aggregate types
    entt::storage<aggregate_type>{}.emplace(entt::entity{0}, 42);
}

TEST(Storage, TypesFromStandardTemplateLibraryMustWork) {
    // see #37 - this test shouldn't crash, that's all
    entt::storage<std::unordered_set<int>> pool;
    pool.emplace(entt::entity{0}).insert(42);
    pool.erase(entt::entity{0});
}

TEST(Storage, Iterator) {
    using iterator = typename entt::storage<boxed_int>::iterator;

    entt::storage<boxed_int> pool;
    pool.emplace(entt::entity{3}, 42);

    iterator end{pool.begin()};
    iterator begin{};
    begin = pool.end();
    std::swap(begin, end);

    ASSERT_EQ(begin, pool.begin());
    ASSERT_EQ(end, pool.end());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, pool.begin());
    ASSERT_EQ(begin--, pool.end());

    ASSERT_EQ(begin+1, pool.end());
    ASSERT_EQ(end-1, pool.begin());

    ASSERT_EQ(++begin, pool.end());
    ASSERT_EQ(--begin, pool.begin());

    ASSERT_EQ(begin += 1, pool.end());
    ASSERT_EQ(begin -= 1, pool.begin());

    ASSERT_EQ(begin + (end - begin), pool.end());
    ASSERT_EQ(begin - (begin - end), pool.end());

    ASSERT_EQ(end - (end - begin), pool.begin());
    ASSERT_EQ(end + (begin - end), pool.begin());

    ASSERT_EQ(begin[0u].value, pool.begin()->value);

    ASSERT_LT(begin, end);
    ASSERT_LE(begin, pool.begin());

    ASSERT_GT(end, begin);
    ASSERT_GE(end, pool.end());
}

TEST(Storage, ConstIterator) {
    using iterator = typename entt::storage<boxed_int>::const_iterator;

    entt::storage<boxed_int> pool;
    pool.emplace(entt::entity{3}, 42);

    iterator cend{pool.cbegin()};
    iterator cbegin{};
    cbegin = pool.cend();
    std::swap(cbegin, cend);

    ASSERT_EQ(cbegin, pool.cbegin());
    ASSERT_EQ(cend, pool.cend());
    ASSERT_NE(cbegin, cend);

    ASSERT_EQ(cbegin++, pool.cbegin());
    ASSERT_EQ(cbegin--, pool.cend());

    ASSERT_EQ(cbegin+1, pool.cend());
    ASSERT_EQ(cend-1, pool.cbegin());

    ASSERT_EQ(++cbegin, pool.cend());
    ASSERT_EQ(--cbegin, pool.cbegin());

    ASSERT_EQ(cbegin += 1, pool.cend());
    ASSERT_EQ(cbegin -= 1, pool.cbegin());

    ASSERT_EQ(cbegin + (cend - cbegin), pool.cend());
    ASSERT_EQ(cbegin - (cbegin - cend), pool.cend());

    ASSERT_EQ(cend - (cend - cbegin), pool.cbegin());
    ASSERT_EQ(cend + (cbegin - cend), pool.cbegin());

    ASSERT_EQ(cbegin[0u].value, pool.cbegin()->value);

    ASSERT_LT(cbegin, cend);
    ASSERT_LE(cbegin, pool.cbegin());

    ASSERT_GT(cend, cbegin);
    ASSERT_GE(cend, pool.cend());
}

TEST(Storage, ReverseIterator) {
    using reverse_iterator = typename entt::storage<boxed_int>::reverse_iterator;

    entt::storage<boxed_int> pool;
    pool.emplace(entt::entity{3}, 42);

    reverse_iterator end{pool.rbegin()};
    reverse_iterator begin{};
    begin = pool.rend();
    std::swap(begin, end);

    ASSERT_EQ(begin, pool.rbegin());
    ASSERT_EQ(end, pool.rend());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, pool.rbegin());
    ASSERT_EQ(begin--, pool.rend());

    ASSERT_EQ(begin+1, pool.rend());
    ASSERT_EQ(end-1, pool.rbegin());

    ASSERT_EQ(++begin, pool.rend());
    ASSERT_EQ(--begin, pool.rbegin());

    ASSERT_EQ(begin += 1, pool.rend());
    ASSERT_EQ(begin -= 1, pool.rbegin());

    ASSERT_EQ(begin + (end - begin), pool.rend());
    ASSERT_EQ(begin - (begin - end), pool.rend());

    ASSERT_EQ(end - (end - begin), pool.rbegin());
    ASSERT_EQ(end + (begin - end), pool.rbegin());

    ASSERT_EQ(begin[0u].value, pool.rbegin()->value);

    ASSERT_LT(begin, end);
    ASSERT_LE(begin, pool.rbegin());

    ASSERT_GT(end, begin);
    ASSERT_GE(end, pool.rend());
}

TEST(Storage, ConstReverseIterator) {
    using const_reverse_iterator = typename entt::storage<boxed_int>::const_reverse_iterator;

    entt::storage<boxed_int> pool;
    pool.emplace(entt::entity{3}, 42);

    const_reverse_iterator cend{pool.crbegin()};
    const_reverse_iterator cbegin{};
    cbegin = pool.crend();
    std::swap(cbegin, cend);

    ASSERT_EQ(cbegin, pool.crbegin());
    ASSERT_EQ(cend, pool.crend());
    ASSERT_NE(cbegin, cend);

    ASSERT_EQ(cbegin++, pool.crbegin());
    ASSERT_EQ(cbegin--, pool.crend());

    ASSERT_EQ(cbegin+1, pool.crend());
    ASSERT_EQ(cend-1, pool.crbegin());

    ASSERT_EQ(++cbegin, pool.crend());
    ASSERT_EQ(--cbegin, pool.crbegin());

    ASSERT_EQ(cbegin += 1, pool.crend());
    ASSERT_EQ(cbegin -= 1, pool.crbegin());

    ASSERT_EQ(cbegin + (cend - cbegin), pool.crend());
    ASSERT_EQ(cbegin - (cbegin - cend), pool.crend());

    ASSERT_EQ(cend - (cend - cbegin), pool.crbegin());
    ASSERT_EQ(cend + (cbegin - cend), pool.crbegin());

    ASSERT_EQ(cbegin[0u].value, pool.crbegin()->value);

    ASSERT_LT(cbegin, cend);
    ASSERT_LE(cbegin, pool.crbegin());

    ASSERT_GT(cend, cbegin);
    ASSERT_GE(cend, pool.crend());
}

TEST(Storage, Raw) {
    entt::storage<int> pool;

    pool.emplace(entt::entity{3}, 3);
    pool.emplace(entt::entity{12}, 6);
    pool.emplace(entt::entity{42}, 9);

    ASSERT_EQ(pool.get(entt::entity{3}), 3);
    ASSERT_EQ(std::as_const(pool).get(entt::entity{12}), 6);
    ASSERT_EQ(pool.get(entt::entity{42}), 9);

    ASSERT_EQ(pool.raw()[0u][0u], 3);
    ASSERT_EQ(std::as_const(pool).raw()[0u][1u], 6);
    ASSERT_EQ(pool.raw()[0u][2u], 9);
}

TEST(Storage, SortOrdered) {
    entt::storage<boxed_int> pool;
    entt::entity entities[5u]{entt::entity{12}, entt::entity{42}, entt::entity{7}, entt::entity{3}, entt::entity{9}};
    boxed_int values[5u]{{12}, {9}, {6}, {3}, {1}};

    pool.insert(std::begin(entities), std::end(entities), values);
    pool.sort([](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    ASSERT_TRUE(std::equal(std::rbegin(entities), std::rend(entities), pool.entt::sparse_set::begin(), pool.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(values), std::rend(values), pool.begin(), pool.end()));
}

TEST(Storage, SortReverse) {
    entt::storage<boxed_int> pool;
    entt::entity entities[5u]{entt::entity{12}, entt::entity{42}, entt::entity{7}, entt::entity{3}, entt::entity{9}};
    boxed_int values[5u]{{1}, {3}, {6}, {9}, {12}};

    pool.insert(std::begin(entities), std::end(entities), values);
    pool.sort([](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    ASSERT_TRUE(std::equal(std::begin(entities), std::end(entities), pool.entt::sparse_set::begin(), pool.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::begin(values), std::end(values), pool.begin(), pool.end()));
}

TEST(Storage, SortUnordered) {
    entt::storage<boxed_int> pool;
    entt::entity entities[5u]{entt::entity{12}, entt::entity{42}, entt::entity{7}, entt::entity{3}, entt::entity{9}};
    boxed_int values[5u]{{6}, {3}, {1}, {9}, {12}};

    pool.insert(std::begin(entities), std::end(entities), values);
    pool.sort([](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    auto begin = pool.begin();
    auto end = pool.end();

    ASSERT_EQ(*(begin++), boxed_int{1});
    ASSERT_EQ(*(begin++), boxed_int{3});
    ASSERT_EQ(*(begin++), boxed_int{6});
    ASSERT_EQ(*(begin++), boxed_int{9});
    ASSERT_EQ(*(begin++), boxed_int{12});
    ASSERT_EQ(begin, end);

    ASSERT_EQ(pool.data()[0u], entt::entity{9});
    ASSERT_EQ(pool.data()[1u], entt::entity{3});
    ASSERT_EQ(pool.data()[2u], entt::entity{12});
    ASSERT_EQ(pool.data()[3u], entt::entity{42});
    ASSERT_EQ(pool.data()[4u], entt::entity{7});
}

TEST(Storage, SortRange) {
    entt::storage<boxed_int> pool;
    entt::entity entities[5u]{entt::entity{12}, entt::entity{42}, entt::entity{7}, entt::entity{3}, entt::entity{9}};
    boxed_int values[5u]{{3}, {6}, {1}, {9}, {12}};

    pool.insert(std::begin(entities), std::end(entities), values);
    pool.sort_n(0u, [](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    ASSERT_TRUE(std::equal(std::rbegin(entities), std::rend(entities), pool.entt::sparse_set::begin(), pool.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(values), std::rend(values), pool.begin(), pool.end()));

    pool.sort_n(2u, [](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    ASSERT_EQ(pool.raw()[0u][0u], boxed_int{6});
    ASSERT_EQ(pool.raw()[0u][1u], boxed_int{3});
    ASSERT_EQ(pool.raw()[0u][2u], boxed_int{1});

    ASSERT_EQ(pool.data()[0u], entt::entity{42});
    ASSERT_EQ(pool.data()[1u], entt::entity{12});
    ASSERT_EQ(pool.data()[2u], entt::entity{7});

    pool.sort_n(5u, [](auto lhs, auto rhs) { return lhs.value < rhs.value; });

    auto begin = pool.begin();
    auto end = pool.end();

    ASSERT_EQ(*(begin++), boxed_int{1});
    ASSERT_EQ(*(begin++), boxed_int{3});
    ASSERT_EQ(*(begin++), boxed_int{6});
    ASSERT_EQ(*(begin++), boxed_int{9});
    ASSERT_EQ(*(begin++), boxed_int{12});
    ASSERT_EQ(begin, end);

    ASSERT_EQ(pool.data()[0u], entt::entity{9});
    ASSERT_EQ(pool.data()[1u], entt::entity{3});
    ASSERT_EQ(pool.data()[2u], entt::entity{42});
    ASSERT_EQ(pool.data()[3u], entt::entity{12});
    ASSERT_EQ(pool.data()[4u], entt::entity{7});
}

TEST(Storage, RespectDisjoint) {
    entt::storage<int> lhs;
    entt::storage<int> rhs;

    entt::entity lhs_entities[3u]{entt::entity{3}, entt::entity{12}, entt::entity{42}};
    int lhs_values[3u]{3, 6, 9};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities), lhs_values);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));

    lhs.respect(rhs);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));
}

TEST(Storage, RespectOverlap) {
    entt::storage<int> lhs;
    entt::storage<int> rhs;

    entt::entity lhs_entities[3u]{entt::entity{3}, entt::entity{12}, entt::entity{42}};
    int lhs_values[3u]{3, 6, 9};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities), lhs_values);

    entt::entity rhs_entities[1u]{entt::entity{12}};
    int rhs_values[1u]{6};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities), rhs_values);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.entt::sparse_set::begin(), rhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_values), std::rend(rhs_values), rhs.begin(), rhs.end()));

    lhs.respect(rhs);

    auto begin = lhs.begin();
    auto end = lhs.end();

    ASSERT_EQ(*(begin++), 6);
    ASSERT_EQ(*(begin++), 9);
    ASSERT_EQ(*(begin++), 3);
    ASSERT_EQ(begin, end);

    ASSERT_EQ(lhs.data()[0u], entt::entity{3});
    ASSERT_EQ(lhs.data()[1u], entt::entity{42});
    ASSERT_EQ(lhs.data()[2u], entt::entity{12});
}

TEST(Storage, RespectOrdered) {
    entt::storage<int> lhs;
    entt::storage<int> rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    int lhs_values[5u]{1, 2, 3, 4, 5};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities), lhs_values);

    entt::entity rhs_entities[6u]{entt::entity{6}, entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    int rhs_values[6u]{6, 1, 2, 3, 4, 5};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities), rhs_values);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.entt::sparse_set::begin(), rhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_values), std::rend(rhs_values), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.entt::sparse_set::begin(), rhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_values), std::rend(rhs_values), rhs.begin(), rhs.end()));
}

TEST(Storage, RespectReverse) {
    entt::storage<int> lhs;
    entt::storage<int> rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    int lhs_values[5u]{1, 2, 3, 4, 5};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities), lhs_values);

    entt::entity rhs_entities[6u]{entt::entity{5}, entt::entity{4}, entt::entity{3}, entt::entity{2}, entt::entity{1}, entt::entity{6}};
    int rhs_values[6u]{5, 4, 3, 2, 1, 6};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities), rhs_values);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.entt::sparse_set::begin(), rhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_values), std::rend(rhs_values), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    auto begin = rhs.begin();
    auto end = rhs.end();

    ASSERT_EQ(*(begin++), 5);
    ASSERT_EQ(*(begin++), 4);
    ASSERT_EQ(*(begin++), 3);
    ASSERT_EQ(*(begin++), 2);
    ASSERT_EQ(*(begin++), 1);
    ASSERT_EQ(*(begin++), 6);
    ASSERT_EQ(begin, end);

    ASSERT_EQ(rhs.data()[0u], entt::entity{6});
    ASSERT_EQ(rhs.data()[1u], entt::entity{1});
    ASSERT_EQ(rhs.data()[2u], entt::entity{2});
    ASSERT_EQ(rhs.data()[3u], entt::entity{3});
    ASSERT_EQ(rhs.data()[4u], entt::entity{4});
    ASSERT_EQ(rhs.data()[5u], entt::entity{5});
}

TEST(Storage, RespectUnordered) {
    entt::storage<int> lhs;
    entt::storage<int> rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    int lhs_values[5u]{1, 2, 3, 4, 5};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities), lhs_values);

    entt::entity rhs_entities[6u]{entt::entity{3}, entt::entity{2}, entt::entity{6}, entt::entity{1}, entt::entity{4}, entt::entity{5}};
    int rhs_values[6u]{3, 2, 6, 1, 4, 5};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities), rhs_values);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.entt::sparse_set::begin(), lhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(lhs_values), std::rend(lhs_values), lhs.begin(), lhs.end()));

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.entt::sparse_set::begin(), rhs.entt::sparse_set::end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_values), std::rend(rhs_values), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    auto begin = rhs.begin();
    auto end = rhs.end();

    ASSERT_EQ(*(begin++), 5);
    ASSERT_EQ(*(begin++), 4);
    ASSERT_EQ(*(begin++), 3);
    ASSERT_EQ(*(begin++), 2);
    ASSERT_EQ(*(begin++), 1);
    ASSERT_EQ(*(begin++), 6);
    ASSERT_EQ(begin, end);

    ASSERT_EQ(rhs.data()[0u], entt::entity{6});
    ASSERT_EQ(rhs.data()[1u], entt::entity{1});
    ASSERT_EQ(rhs.data()[2u], entt::entity{2});
    ASSERT_EQ(rhs.data()[3u], entt::entity{3});
    ASSERT_EQ(rhs.data()[4u], entt::entity{4});
    ASSERT_EQ(rhs.data()[5u], entt::entity{5});
}

TEST(Storage, CanModifyDuringIteration) {
    entt::storage<int> pool;
    pool.emplace(entt::entity{0}, 42);

    ASSERT_EQ(pool.capacity(), ENTT_PACKED_PAGE);

    const auto it = pool.cbegin();
    pool.reserve(ENTT_PACKED_PAGE + 1u);

    ASSERT_EQ(pool.capacity(), 2 * ENTT_PACKED_PAGE);

    // this should crash with asan enabled if we break the constraint
    const auto entity = *it;
    (void)entity;
}

TEST(Storage, ReferencesGuaranteed) {
    entt::storage<boxed_int> pool;

    pool.emplace(entt::entity{0}, 0);
    pool.emplace(entt::entity{1}, 1);

    ASSERT_EQ(pool.get(entt::entity{0}).value, 0);
    ASSERT_EQ(pool.get(entt::entity{1}).value, 1);

    for(auto &&type: pool) {
        if(type.value) {
            type.value = 42;
        }
    }

    ASSERT_EQ(pool.get(entt::entity{0}).value, 0);
    ASSERT_EQ(pool.get(entt::entity{1}).value, 42);

    auto begin = pool.begin();

    while(begin != pool.end()) {
        (begin++)->value = 3;
    }

    ASSERT_EQ(pool.get(entt::entity{0}).value, 3);
    ASSERT_EQ(pool.get(entt::entity{1}).value, 3);
}

TEST(Storage, MoveOnlyComponent) {
    // the purpose is to ensure that move only components are always accepted
    entt::storage<std::unique_ptr<int>> pool;
    (void)pool;
}

TEST(Storage, UpdateFromDestructor) {
    static constexpr auto size = 10u;

    auto test = [](const auto target) {
        entt::storage<update_from_destructor> pool;

        for(std::size_t next{}; next < size; ++next) {
            const auto entity = entt::entity(next);
            pool.emplace(entity, pool, entity == entt::entity(size/2) ? target : entity);
        }

        pool.erase(entt::entity(size/2));

        ASSERT_EQ(pool.size(), size - 1u - (target != entt::null));
        ASSERT_FALSE(pool.contains(entt::entity(size/2)));
        ASSERT_FALSE(pool.contains(target));

        pool.clear();

        ASSERT_TRUE(pool.empty());

        for(std::size_t next{}; next < size; ++next) {
            ASSERT_FALSE(pool.contains(entt::entity(next)));
        }
    };

    test(entt::entity(size - 1u));
    test(entt::entity(size - 2u));
    test(entt::entity{0u});
}

TEST(Storage, ThrowingComponent) {
    entt::storage<test::throwing_component> pool;
    test::throwing_component::trigger_on_value = 42;

    // strong exception safety
    ASSERT_THROW(pool.emplace(entt::entity{0}, test::throwing_component{42}), typename test::throwing_component::exception_type);
    ASSERT_TRUE(pool.empty());

    const entt::entity entities[2u]{entt::entity{42}, entt::entity{1}};
    const test::throwing_component components[2u]{42, 1};

    // basic exception safety
    ASSERT_THROW(pool.insert(std::begin(entities), std::end(entities), test::throwing_component{42}), typename test::throwing_component::exception_type);
    ASSERT_EQ(pool.size(), 0u);
    ASSERT_FALSE(pool.contains(entt::entity{1}));

    // basic exception safety
    ASSERT_THROW(pool.insert(std::begin(entities), std::end(entities), std::begin(components)), typename test::throwing_component::exception_type);
    ASSERT_EQ(pool.size(), 0u);
    ASSERT_FALSE(pool.contains(entt::entity{1}));

    // basic exception safety
    ASSERT_THROW(pool.insert(std::rbegin(entities), std::rend(entities), std::rbegin(components)), typename test::throwing_component::exception_type);
    ASSERT_EQ(pool.size(), 1u);
    ASSERT_TRUE(pool.contains(entt::entity{1}));
    ASSERT_EQ(pool.get(entt::entity{1}), 1);

    pool.clear();
    pool.emplace(entt::entity{1}, 1);
    pool.emplace(entt::entity{42}, 42);

    // basic exception safety
    ASSERT_THROW(pool.erase(entt::entity{1}), typename test::throwing_component::exception_type);
    ASSERT_EQ(pool.size(), 2u);
    ASSERT_TRUE(pool.contains(entt::entity{42}));
    ASSERT_TRUE(pool.contains(entt::entity{1}));
    ASSERT_EQ(pool.at(0u), entt::entity{1});
    ASSERT_EQ(pool.at(1u), entt::entity{42});
    ASSERT_EQ(pool.get(entt::entity{42}), 42);
    // the element may have been moved but it's still there
    ASSERT_EQ(pool.get(entt::entity{1}), test::throwing_component::moved_from_value);

    test::throwing_component::trigger_on_value = 99;
    pool.erase(entt::entity{1});

    ASSERT_EQ(pool.size(), 1u);
    ASSERT_TRUE(pool.contains(entt::entity{42}));
    ASSERT_FALSE(pool.contains(entt::entity{1}));
    ASSERT_EQ(pool.at(0u), entt::entity{42});
    ASSERT_EQ(pool.get(entt::entity{42}), 42);
}

TEST(Storage, ThrowingAllocator) {
    entt::basic_storage<entt::entity, int, test::throwing_allocator<int>> pool;

    test::throwing_allocator<int>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(pool.reserve(1u), test::throwing_allocator<int>::exception_type);
    ASSERT_EQ(pool.capacity(), 0u);

    test::throwing_allocator<int>::trigger_after_allocate = true;

    // strong exception safety
    ASSERT_THROW(pool.reserve(2 * ENTT_PACKED_PAGE), test::throwing_allocator<int>::exception_type);
    ASSERT_EQ(pool.capacity(), 0u);

    pool.shrink_to_fit();
    test::throwing_allocator<int>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(pool.emplace(entt::entity{0}, 0), test::throwing_allocator<int>::exception_type);
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_TRUE(pool.empty());

    test::throwing_allocator<entt::entity>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(pool.emplace(entt::entity{0}, 0), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_FALSE(pool.contains(entt::entity{0}));
    ASSERT_TRUE(pool.empty());

    pool.emplace(entt::entity{0}, 0);
    const entt::entity entities[2u]{entt::entity{1}, entt::entity{ENTT_SPARSE_PAGE}};
    test::throwing_allocator<entt::entity>::trigger_after_allocate = true;

    // basic exception safety
    ASSERT_THROW(pool.insert(std::begin(entities), std::end(entities), 0), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_TRUE(pool.contains(entt::entity{1}));
    ASSERT_FALSE(pool.contains(entt::entity{ENTT_SPARSE_PAGE}));

    pool.erase(entt::entity{1});
    const int components[2u]{1, ENTT_SPARSE_PAGE};
    test::throwing_allocator<entt::entity>::trigger_on_allocate = true;

    // basic exception safety
    ASSERT_THROW(pool.insert(std::begin(entities), std::end(entities), std::begin(components)), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_TRUE(pool.contains(entt::entity{1}));
    ASSERT_FALSE(pool.contains(entt::entity{ENTT_SPARSE_PAGE}));
}
