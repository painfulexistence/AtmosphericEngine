#include <cstdint>
#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <gtest/gtest.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/sparse_set.hpp>
#include <entt/entity/fwd.hpp>
#include "throwing_allocator.hpp"

struct empty_type {};
struct boxed_int { int value; };

TEST(SparseSet, Functionalities) {
    entt::sparse_set set;

    set.reserve(42);

    ASSERT_EQ(set.capacity(), 42u);
    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);
    ASSERT_EQ(std::as_const(set).begin(), std::as_const(set).end());
    ASSERT_EQ(set.begin(), set.end());
    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_FALSE(set.contains(entt::entity{42}));

    set.reserve(0);

    ASSERT_EQ(set.capacity(), 42u);
    ASSERT_TRUE(set.empty());

    ASSERT_EQ(set.emplace(entt::entity{42}), 0u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 1u);
    ASSERT_NE(std::as_const(set).begin(), std::as_const(set).end());
    ASSERT_NE(set.begin(), set.end());
    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_TRUE(set.contains(entt::entity{42}));
    ASSERT_EQ(set.index(entt::entity{42}), 0u);
    ASSERT_EQ(set.at(0u), entt::entity{42});
    ASSERT_EQ(set.at(1u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(set[0u], entt::entity{42});

    set.erase(entt::entity{42});

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);
    ASSERT_EQ(std::as_const(set).begin(), std::as_const(set).end());
    ASSERT_EQ(set.begin(), set.end());
    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_FALSE(set.contains(entt::entity{42}));
    ASSERT_EQ(set.at(0u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(set.at(1u), static_cast<entt::entity>(entt::null));

    ASSERT_EQ(set.emplace(entt::entity{42}), 0u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.index(entt::entity{42}), 0u);
    ASSERT_EQ(set.at(0u), entt::entity{42});
    ASSERT_EQ(set.at(1u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(set[0u], entt::entity{42});

    set.clear();

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);
    ASSERT_EQ(std::as_const(set).begin(), std::as_const(set).end());
    ASSERT_EQ(set.begin(), set.end());
    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_FALSE(set.contains(entt::entity{42}));
}

TEST(SparseSet, Contains) {
    entt::sparse_set set{entt::deletion_policy::in_place};

    set.emplace(entt::entity{0});
    set.emplace(entt::entity{3});
    set.emplace(entt::entity{42});
    set.emplace(entt::entity{99});

    set.emplace(entt::entity{1});

    ASSERT_TRUE(set.contains(entt::entity{0}));
    ASSERT_TRUE(set.contains(entt::entity{3}));
    ASSERT_TRUE(set.contains(entt::entity{42}));
    ASSERT_TRUE(set.contains(entt::entity{99}));
    ASSERT_TRUE(set.contains(entt::entity{1}));

    set.erase(entt::entity{0});
    set.erase(entt::entity{3});

    set.remove(entt::entity{42});
    set.remove(entt::entity{99});

    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_FALSE(set.contains(entt::entity{3}));
    ASSERT_FALSE(set.contains(entt::entity{42}));
    ASSERT_FALSE(set.contains(entt::entity{99}));
    ASSERT_TRUE(set.contains(entt::entity{1}));

    ASSERT_DEATH(static_cast<void>(set.contains(entt::null)), "");
    ASSERT_DEATH(static_cast<void>(set.contains(entt::tombstone)), "");
    ASSERT_DEATH(static_cast<void>(set.contains(entt::tombstone | entt::entity{1u})), "");
    ASSERT_DEATH(static_cast<void>(set.contains(entt::null | entt::entity{1u})), "");
}

TEST(SparseSet, Move) {
    entt::sparse_set set;
    set.emplace(entt::entity{42});

    ASSERT_TRUE(std::is_move_constructible_v<decltype(set)>);
    ASSERT_TRUE(std::is_move_assignable_v<decltype(set)>);

    entt::sparse_set other{std::move(set)};

    ASSERT_TRUE(set.empty());
    ASSERT_FALSE(other.empty());
    ASSERT_EQ(set.at(0u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(other.at(0u), entt::entity{42});

    set = std::move(other);

    ASSERT_FALSE(set.empty());
    ASSERT_TRUE(other.empty());
    ASSERT_EQ(set.at(0u), entt::entity{42});
    ASSERT_EQ(other.at(0u), static_cast<entt::entity>(entt::null));

    other = entt::sparse_set{};
    other.emplace(entt::entity{3});
    other = std::move(set);

    ASSERT_TRUE(set.empty());
    ASSERT_FALSE(other.empty());
    ASSERT_EQ(set.at(0u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(other.at(0u), entt::entity{42});
}

TEST(SparseSet, Pagination) {
    entt::sparse_set set;

    ASSERT_EQ(set.extent(), 0u);

    set.emplace(entt::entity{ENTT_SPARSE_PAGE-1u});

    ASSERT_EQ(set.extent(), ENTT_SPARSE_PAGE);
    ASSERT_TRUE(set.contains(entt::entity{ENTT_SPARSE_PAGE-1u}));

    set.emplace(entt::entity{ENTT_SPARSE_PAGE});

    ASSERT_EQ(set.extent(), 2 * ENTT_SPARSE_PAGE);
    ASSERT_TRUE(set.contains(entt::entity{ENTT_SPARSE_PAGE-1u}));
    ASSERT_TRUE(set.contains(entt::entity{ENTT_SPARSE_PAGE}));
    ASSERT_FALSE(set.contains(entt::entity{ENTT_SPARSE_PAGE+1u}));

    set.erase(entt::entity{ENTT_SPARSE_PAGE-1u});

    ASSERT_EQ(set.extent(), 2 * ENTT_SPARSE_PAGE);
    ASSERT_FALSE(set.contains(entt::entity{ENTT_SPARSE_PAGE-1u}));
    ASSERT_TRUE(set.contains(entt::entity{ENTT_SPARSE_PAGE}));

    set.shrink_to_fit();
    set.erase(entt::entity{ENTT_SPARSE_PAGE});

    ASSERT_EQ(set.extent(), 2 * ENTT_SPARSE_PAGE);
    ASSERT_FALSE(set.contains(entt::entity{ENTT_SPARSE_PAGE-1u}));
    ASSERT_FALSE(set.contains(entt::entity{ENTT_SPARSE_PAGE}));

    set.shrink_to_fit();

    ASSERT_EQ(set.extent(), 2 * ENTT_SPARSE_PAGE);
}

TEST(SparseSet, Emplace) {
    entt::sparse_set set{entt::deletion_policy::in_place};
    entt::entity entities[2u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};

    ASSERT_TRUE(set.empty());

    set.emplace(entities[0u]);
    set.erase(entities[0u]);

    set.emplace_back(entities[0u]);
    set.emplace(entities[1u]);

    ASSERT_DEATH(set.emplace_back(entities[1u]), "");
    ASSERT_DEATH(set.emplace(entities[0u]), "");

    ASSERT_EQ(set.at(0u), entities[1u]);
    ASSERT_EQ(set.at(1u), entities[0u]);
    ASSERT_EQ(set.index(entities[0u]), 1u);
    ASSERT_EQ(set.index(entities[1u]), 0u);

    set.erase(std::begin(entities), std::end(entities));
    set.emplace(entities[1u]);
    set.emplace_back(entities[0u]);

    ASSERT_EQ(set.at(0u), entities[1u]);
    ASSERT_EQ(set.at(1u), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(set.at(2u), entities[0u]);
    ASSERT_EQ(set.index(entities[0u]), 2u);
    ASSERT_EQ(set.index(entities[1u]), 0u);
}

TEST(SparseSet, EmplaceOutOfBounds) {
    entt::sparse_set set{entt::deletion_policy::in_place};
    entt::entity entities[2u]{entt::entity{0}, entt::entity{ENTT_SPARSE_PAGE}};
    
    ASSERT_EQ(set.emplace(entities[0u]), 0u);
    ASSERT_EQ(set.extent(), ENTT_SPARSE_PAGE);

    set.erase(entities[0u]);

    ASSERT_EQ(set.emplace(entities[1u]), 0u);
    ASSERT_EQ(set.extent(), 2u * ENTT_SPARSE_PAGE);
}

TEST(SparseSet, Insert) {
    entt::sparse_set set{entt::deletion_policy::in_place};
    entt::entity entities[2u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};

    set.emplace(entt::entity{12});
    set.insert(std::end(entities), std::end(entities));
    set.insert(std::begin(entities), std::end(entities));
    set.emplace(entt::entity{24});

    ASSERT_TRUE(set.contains(entities[0u]));
    ASSERT_TRUE(set.contains(entities[1u]));
    ASSERT_FALSE(set.contains(entt::entity{0}));
    ASSERT_FALSE(set.contains(entt::entity{9}));
    ASSERT_TRUE(set.contains(entt::entity{12}));
    ASSERT_TRUE(set.contains(entt::entity{24}));

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 4u);
    ASSERT_EQ(set.index(entt::entity{12}), 0u);
    ASSERT_EQ(set.index(entities[0u]), 1u);
    ASSERT_EQ(set.index(entities[1u]), 2u);
    ASSERT_EQ(set.index(entt::entity{24}), 3u);
    ASSERT_EQ(set.data()[set.index(entt::entity{12})], entt::entity{12});
    ASSERT_EQ(set.data()[set.index(entities[0u])], entities[0u]);
    ASSERT_EQ(set.data()[set.index(entities[1u])], entities[1u]);
    ASSERT_EQ(set.data()[set.index(entt::entity{24})], entt::entity{24});

    set.erase(std::begin(entities), std::end(entities));
    set.insert(std::rbegin(entities), std::rend(entities));

    ASSERT_EQ(set.size(), 6u);
    ASSERT_TRUE(set.at(1u) == entt::tombstone);
    ASSERT_TRUE(set.at(2u) == entt::tombstone);
    ASSERT_EQ(set.at(4u), entities[1u]);
    ASSERT_EQ(set.at(5u), entities[0u]);
    ASSERT_EQ(set.index(entities[0u]), 5u);
    ASSERT_EQ(set.index(entities[1u]), 4u);
}

TEST(SparseSet, Erase) {
    entt::sparse_set set;
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    ASSERT_EQ(set.policy(), entt::deletion_policy::swap_and_pop);
    ASSERT_TRUE(set.empty());

    ASSERT_DEATH(set.erase(std::begin(entities), std::end(entities)), "");
    ASSERT_DEATH(set.erase(entities[1u]), "");

    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));
    set.erase(set.begin(), set.end());

    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));
    set.erase(entities, entities + 2u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(*set.begin(), entities[2u]);

    set.erase(entities[2u]);

    ASSERT_DEATH(set.erase(entities[2u]), "");
    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));
    std::swap(entities[1u], entities[2u]);
    set.erase(entities, entities + 2u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(*set.begin(), entities[2u]);
}

TEST(SparseSet, StableErase) {
    entt::sparse_set set{entt::deletion_policy::in_place};
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    ASSERT_EQ(set.policy(), entt::deletion_policy::in_place);
    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    ASSERT_DEATH(set.erase(std::begin(entities), std::end(entities)), "");
    ASSERT_DEATH(set.erase(entities[1u]), "");

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    set.insert(std::begin(entities), std::end(entities));
    set.erase(set.begin(), set.end());

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 3u);
    ASSERT_TRUE(set.at(0u) == entt::tombstone);
    ASSERT_TRUE(set.at(1u) == entt::tombstone);
    ASSERT_TRUE(set.at(2u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 0u);

    set.insert(std::begin(entities), std::end(entities));
    set.erase(entities, entities + 2u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 6u);
    ASSERT_EQ(*set.begin(), entities[2u]);
    ASSERT_TRUE(set.at(3u) == entt::tombstone);
    ASSERT_TRUE(set.at(4u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 4u);

    set.erase(entities[2u]);

    ASSERT_DEATH(set.erase(entities[2u]), "");
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 6u);
    ASSERT_EQ(set.slot(), 5u);

    set.insert(std::begin(entities), std::end(entities));
    std::swap(entities[1u], entities[2u]);
    set.erase(entities, entities + 2u);

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 9u);
    ASSERT_TRUE(set.at(6u) == entt::tombstone);
    ASSERT_EQ(set.at(7u), entities[2u]);
    ASSERT_EQ(*++set.begin(), entities[2u]);
    ASSERT_TRUE(set.at(8u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 8u);

    set.compact();

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 1u);
    ASSERT_EQ(*set.begin(), entities[2u]);
    ASSERT_EQ(set.slot(), 1u);

    set.clear();

    ASSERT_EQ(set.size(), 0u);
    ASSERT_EQ(set.slot(), 0u);

    set.insert(std::begin(entities), std::end(entities));
    set.erase(entities[2u]);

    ASSERT_DEATH(set.erase(entities[2u]), "");
    ASSERT_EQ(set.slot(), 2u);

    set.erase(entities[0u]);
    set.erase(entities[1u]);

    ASSERT_DEATH(set.erase(entities, entities + 2u), "");
    ASSERT_EQ(set.size(), 3u);
    ASSERT_TRUE(*set.begin() == entt::tombstone);
    ASSERT_EQ(set.slot(), 1u);

    ASSERT_EQ(set.emplace(entities[0u]), 1u);
    ASSERT_EQ(*++set.begin(), entities[0u]);

    ASSERT_EQ(set.emplace(entities[1u]), 0u);
    ASSERT_EQ(set.emplace(entities[2u]), 2u);
    ASSERT_EQ(set.emplace(entt::entity{0}), 3u);

    ASSERT_EQ(set.size(), 4u);
    ASSERT_EQ(*set.begin(), entt::entity{0});
    ASSERT_EQ(set.at(0u), entities[1u]);
    ASSERT_EQ(set.at(1u), entities[0u]);
    ASSERT_EQ(set.at(2u), entities[2u]);
}

TEST(SparseSet, Remove) {
    entt::sparse_set set;
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    ASSERT_EQ(set.policy(), entt::deletion_policy::swap_and_pop);
    ASSERT_TRUE(set.empty());

    ASSERT_EQ(set.remove(std::begin(entities), std::end(entities)), 0u);
    ASSERT_EQ(set.remove(entities[1u]), 0u);

    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));

    ASSERT_EQ(set.remove(set.begin(), set.end()), 3u);
    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));

    ASSERT_EQ(set.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(*set.begin(), entities[2u]);

    ASSERT_EQ(set.remove(entities[2u]), 1u);
    ASSERT_EQ(set.remove(entities[2u]), 0u);
    ASSERT_TRUE(set.empty());

    set.insert(entities, entities + 2u);

    ASSERT_EQ(set.remove(std::begin(entities), std::end(entities)), 2u);
    ASSERT_TRUE(set.empty());

    set.insert(std::begin(entities), std::end(entities));
    std::swap(entities[1u], entities[2u]);

    ASSERT_EQ(set.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(*set.begin(), entities[2u]);
}

TEST(SparseSet, StableRemove) {
    entt::sparse_set set{entt::deletion_policy::in_place};
    entt::entity entities[3u];

    entities[0u] = entt::entity{3};
    entities[1u] = entt::entity{42};
    entities[2u] = entt::entity{9};

    ASSERT_EQ(set.policy(), entt::deletion_policy::in_place);
    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    ASSERT_EQ(set.remove(std::begin(entities), std::end(entities)), 0u);
    ASSERT_EQ(set.remove(entities[1u]), 0u);

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    set.insert(std::begin(entities), std::end(entities));

    ASSERT_EQ(set.remove(set.begin(), set.end()), 3u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 3u);
    ASSERT_TRUE(set.at(0u) == entt::tombstone);
    ASSERT_TRUE(set.at(1u) == entt::tombstone);
    ASSERT_TRUE(set.at(2u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 0u);

    set.insert(std::begin(entities), std::end(entities));

    ASSERT_EQ(set.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 6u);
    ASSERT_EQ(*set.begin(), entt::entity{9});
    ASSERT_TRUE(set.at(3u) == entt::tombstone);
    ASSERT_TRUE(set.at(4u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 4u);

    ASSERT_EQ(set.remove(entities[2u]), 1u);
    ASSERT_EQ(set.remove(entities[2u]), 0u);
    ASSERT_EQ(set.remove(entities[2u]), 0u);
    ASSERT_EQ(set.remove(entities[2u]), 0u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 6u);
    ASSERT_TRUE(*set.begin() == entt::tombstone);
    ASSERT_EQ(set.slot(), 5u);

    set.insert(entities, entities + 2u);

    ASSERT_EQ(set.remove(std::begin(entities), std::end(entities)), 2u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 8u);
    ASSERT_TRUE(set.at(6u) == entt::tombstone);
    ASSERT_TRUE(set.at(7u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 7u);

    set.insert(std::begin(entities), std::end(entities));
    std::swap(entities[1u], entities[2u]);

    ASSERT_EQ(set.remove(entities, entities + 2u), 2u);
    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 11u);
    ASSERT_TRUE(set.at(8u) == entt::tombstone);
    ASSERT_EQ(set.at(9u), entities[2u]);
    ASSERT_EQ(*++set.begin(), entities[2u]);
    ASSERT_TRUE(set.at(10u) == entt::tombstone);
    ASSERT_EQ(set.slot(), 10u);

    set.compact();

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 1u);
    ASSERT_EQ(*set.begin(), entities[2u]);
    ASSERT_EQ(set.slot(), 1u);

    set.clear();

    ASSERT_EQ(set.size(), 0u);
    ASSERT_EQ(set.slot(), 0u);

    set.insert(std::begin(entities), std::end(entities));

    ASSERT_EQ(set.remove(entities[2u]), 1u);
    ASSERT_EQ(set.remove(entities[2u]), 0u);

    ASSERT_EQ(set.remove(entities[0u]), 1u);
    ASSERT_EQ(set.remove(entities[1u]), 1u);
    ASSERT_EQ(set.remove(entities, entities + 2u), 0u);

    ASSERT_EQ(set.size(), 3u);
    ASSERT_TRUE(*set.begin() == entt::tombstone);
    ASSERT_EQ(set.slot(), 1u);

    ASSERT_EQ(set.emplace(entities[0u]), 1u);
    ASSERT_EQ(*++set.begin(), entities[0u]);

    ASSERT_EQ(set.emplace(entities[1u]), 0u);
    ASSERT_EQ(set.emplace(entities[2u]), 2u);
    ASSERT_EQ(set.emplace(entt::entity{0}), 3u);

    ASSERT_EQ(set.size(), 4u);
    ASSERT_EQ(*set.begin(), entt::entity{0});
    ASSERT_EQ(set.at(0u), entities[1u]);
    ASSERT_EQ(set.at(1u), entities[0u]);
    ASSERT_EQ(set.at(2u), entities[2u]);
}

TEST(SparseSet, Compact) {
    entt::sparse_set set{entt::deletion_policy::in_place};

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    set.compact();

    ASSERT_TRUE(set.empty());
    ASSERT_EQ(set.size(), 0u);

    set.emplace(entt::entity{0});
    set.compact();

    ASSERT_FALSE(set.empty());
    ASSERT_EQ(set.size(), 1u);

    set.emplace(entt::entity{42});
    set.erase(entt::entity{0});

    ASSERT_EQ(set.size(), 2u);
    ASSERT_EQ(set.index(entt::entity{42}), 1u);

    set.compact();

    ASSERT_EQ(set.size(), 1u);
    ASSERT_EQ(set.index(entt::entity{42}), 0u);

    set.emplace(entt::entity{0});
    set.compact();

    ASSERT_EQ(set.size(), 2u);
    ASSERT_EQ(set.index(entt::entity{42}), 0u);
    ASSERT_EQ(set.index(entt::entity{0}), 1u);

    set.erase(entt::entity{0});
    set.erase(entt::entity{42});
    set.compact();

    ASSERT_TRUE(set.empty());
}

TEST(SparseSet, Clear) {
    entt::sparse_set set;

    set.emplace(entt::entity{3});
    set.emplace(entt::entity{42});
    set.emplace(entt::entity{9});

    ASSERT_FALSE(set.empty());

    set.clear();

    ASSERT_TRUE(set.empty());
}

TEST(SparseSet, Iterator) {
    using iterator = typename entt::sparse_set::iterator;

    entt::sparse_set set;
    set.emplace(entt::entity{3});

    iterator end{set.begin()};
    iterator begin{};
    begin = set.end();
    std::swap(begin, end);

    ASSERT_EQ(begin, set.begin());
    ASSERT_EQ(end, set.end());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, set.begin());
    ASSERT_EQ(begin--, set.end());

    ASSERT_EQ(begin+1, set.end());
    ASSERT_EQ(end-1, set.begin());

    ASSERT_EQ(++begin, set.end());
    ASSERT_EQ(--begin, set.begin());

    ASSERT_EQ(begin += 1, set.end());
    ASSERT_EQ(begin -= 1, set.begin());

    ASSERT_EQ(begin + (end - begin), set.end());
    ASSERT_EQ(begin - (begin - end), set.end());

    ASSERT_EQ(end - (end - begin), set.begin());
    ASSERT_EQ(end + (begin - end), set.begin());

    ASSERT_EQ(begin[0u], *set.begin());

    ASSERT_LT(begin, end);
    ASSERT_LE(begin, set.begin());

    ASSERT_GT(end, begin);
    ASSERT_GE(end, set.end());

    ASSERT_EQ(*begin, entt::entity{3});
    ASSERT_EQ(*begin.operator->(), entt::entity{3});
}

TEST(SparseSet, ReverseIterator) {
    using reverse_iterator = typename entt::sparse_set::reverse_iterator;

    entt::sparse_set set;
    set.emplace(entt::entity{3});

    reverse_iterator end{set.rbegin()};
    reverse_iterator begin{};
    begin = set.rend();
    std::swap(begin, end);

    ASSERT_EQ(begin, set.rbegin());
    ASSERT_EQ(end, set.rend());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, set.rbegin());
    ASSERT_EQ(begin--, set.rend());

    ASSERT_EQ(begin+1, set.rend());
    ASSERT_EQ(end-1, set.rbegin());

    ASSERT_EQ(++begin, set.rend());
    ASSERT_EQ(--begin, set.rbegin());

    ASSERT_EQ(begin += 1, set.rend());
    ASSERT_EQ(begin -= 1, set.rbegin());

    ASSERT_EQ(begin + (end - begin), set.rend());
    ASSERT_EQ(begin - (begin - end), set.rend());

    ASSERT_EQ(end - (end - begin), set.rbegin());
    ASSERT_EQ(end + (begin - end), set.rbegin());

    ASSERT_EQ(begin[0u], *set.rbegin());

    ASSERT_LT(begin, end);
    ASSERT_LE(begin, set.rbegin());

    ASSERT_GT(end, begin);
    ASSERT_GE(end, set.rend());

    ASSERT_EQ(*begin, entt::entity{3});
}

TEST(SparseSet, Find) {
    entt::sparse_set set;
    set.emplace(entt::entity{3});
    set.emplace(entt::entity{42});
    set.emplace(entt::entity{99});

    ASSERT_NE(set.find(entt::entity{3}), set.end());
    ASSERT_NE(set.find(entt::entity{42}), set.end());
    ASSERT_NE(set.find(entt::entity{99}), set.end());
    ASSERT_EQ(set.find(entt::entity{0}), set.end());

    auto it = set.find(entt::entity{99});

    ASSERT_EQ(*it, entt::entity{99});
    ASSERT_EQ(*(++it), entt::entity{42});
    ASSERT_EQ(*(++it), entt::entity{3});
    ASSERT_EQ(++it, set.end());
    ASSERT_EQ(++set.find(entt::entity{3}), set.end());
}

TEST(SparseSet, Data) {
    entt::sparse_set set;

    set.emplace(entt::entity{3});
    set.emplace(entt::entity{12});
    set.emplace(entt::entity{42});

    ASSERT_EQ(set.index(entt::entity{3}), 0u);
    ASSERT_EQ(set.index(entt::entity{12}), 1u);
    ASSERT_EQ(set.index(entt::entity{42}), 2u);

    ASSERT_EQ(set.data()[0u], entt::entity{3});
    ASSERT_EQ(set.data()[1u], entt::entity{12});
    ASSERT_EQ(set.data()[2u], entt::entity{42});
}

TEST(SparseSet, SortOrdered) {
    entt::sparse_set set;
    entt::entity entities[5u]{entt::entity{42}, entt::entity{12}, entt::entity{9}, entt::entity{7}, entt::entity{3}};

    set.insert(std::begin(entities), std::end(entities));
    set.sort(std::less{});

    ASSERT_TRUE(std::equal(std::rbegin(entities), std::rend(entities), set.begin(), set.end()));
}

TEST(SparseSet, SortReverse) {
    entt::sparse_set set;
    entt::entity entities[5u]{entt::entity{3}, entt::entity{7}, entt::entity{9}, entt::entity{12}, entt::entity{42}};

    set.insert(std::begin(entities), std::end(entities));
    set.sort(std::less{});

    ASSERT_TRUE(std::equal(std::begin(entities), std::end(entities), set.begin(), set.end()));
}

TEST(SparseSet, SortUnordered) {
    entt::sparse_set set;
    entt::entity entities[5u]{entt::entity{9}, entt::entity{7}, entt::entity{3}, entt::entity{12}, entt::entity{42}};

    set.insert(std::begin(entities), std::end(entities));
    set.sort(std::less{});

    auto begin = set.begin();
    auto end = set.end();

    ASSERT_EQ(*(begin++), entt::entity{3});
    ASSERT_EQ(*(begin++), entt::entity{7});
    ASSERT_EQ(*(begin++), entt::entity{9});
    ASSERT_EQ(*(begin++), entt::entity{12});
    ASSERT_EQ(*(begin++), entt::entity{42});
    ASSERT_EQ(begin, end);
}

TEST(SparseSet, SortRange) {
    entt::sparse_set set;
    entt::entity entities[5u]{entt::entity{7}, entt::entity{9}, entt::entity{3}, entt::entity{12}, entt::entity{42}};

    set.insert(std::begin(entities), std::end(entities));
    set.sort_n(0u, std::less{});

    ASSERT_TRUE(std::equal(std::rbegin(entities), std::rend(entities), set.begin(), set.end()));

    set.sort_n(2u, std::less{});

    ASSERT_EQ(set.data()[0u], entt::entity{9});
    ASSERT_EQ(set.data()[1u], entt::entity{7});
    ASSERT_EQ(set.data()[2u], entt::entity{3});

    set.sort_n(5u, std::less{});

    auto begin = set.begin();
    auto end = set.end();

    ASSERT_EQ(*(begin++), entt::entity{3});
    ASSERT_EQ(*(begin++), entt::entity{7});
    ASSERT_EQ(*(begin++), entt::entity{9});
    ASSERT_EQ(*(begin++), entt::entity{12});
    ASSERT_EQ(*(begin++), entt::entity{42});
    ASSERT_EQ(begin, end);
}

TEST(SparseSet, RespectDisjoint) {
    entt::sparse_set lhs;
    entt::sparse_set rhs;

    entt::entity lhs_entities[3u]{entt::entity{3}, entt::entity{12}, entt::entity{42}};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities));

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));

    lhs.respect(rhs);

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));
}

TEST(SparseSet, RespectOverlap) {
    entt::sparse_set lhs;
    entt::sparse_set rhs;

    entt::entity lhs_entities[3u]{entt::entity{3}, entt::entity{12}, entt::entity{42}};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities));

    entt::entity rhs_entities[1u]{entt::entity{12}};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities));

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.begin(), rhs.end()));

    lhs.respect(rhs);

    auto begin = lhs.begin();
    auto end = lhs.end();

    ASSERT_EQ(*(begin++), entt::entity{12});
    ASSERT_EQ(*(begin++), entt::entity{42});
    ASSERT_EQ(*(begin++), entt::entity{3});
    ASSERT_EQ(begin, end);
}

TEST(SparseSet, RespectOrdered) {
    entt::sparse_set lhs;
    entt::sparse_set rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities));

    entt::entity rhs_entities[6u]{entt::entity{6}, entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities));

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.begin(), rhs.end()));
}

TEST(SparseSet, RespectReverse) {
    entt::sparse_set lhs;
    entt::sparse_set rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities));

    entt::entity rhs_entities[6u]{entt::entity{5}, entt::entity{4}, entt::entity{3}, entt::entity{2}, entt::entity{1}, entt::entity{6}};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities));

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    auto begin = rhs.begin();
    auto end = rhs.end();

    ASSERT_EQ(*(begin++), entt::entity{5});
    ASSERT_EQ(*(begin++), entt::entity{4});
    ASSERT_EQ(*(begin++), entt::entity{3});
    ASSERT_EQ(*(begin++), entt::entity{2});
    ASSERT_EQ(*(begin++), entt::entity{1});
    ASSERT_EQ(*(begin++), entt::entity{6});
    ASSERT_EQ(begin, end);
}

TEST(SparseSet, RespectUnordered) {
    entt::sparse_set lhs;
    entt::sparse_set rhs;

    entt::entity lhs_entities[5u]{entt::entity{1}, entt::entity{2}, entt::entity{3}, entt::entity{4}, entt::entity{5}};
    lhs.insert(std::begin(lhs_entities), std::end(lhs_entities));

    entt::entity rhs_entities[6u]{entt::entity{3}, entt::entity{2}, entt::entity{6}, entt::entity{1}, entt::entity{4}, entt::entity{5}};
    rhs.insert(std::begin(rhs_entities), std::end(rhs_entities));

    ASSERT_TRUE(std::equal(std::rbegin(lhs_entities), std::rend(lhs_entities), lhs.begin(), lhs.end()));
    ASSERT_TRUE(std::equal(std::rbegin(rhs_entities), std::rend(rhs_entities), rhs.begin(), rhs.end()));

    rhs.respect(lhs);

    auto begin = rhs.begin();
    auto end = rhs.end();

    ASSERT_EQ(*(begin++), entt::entity{5});
    ASSERT_EQ(*(begin++), entt::entity{4});
    ASSERT_EQ(*(begin++), entt::entity{3});
    ASSERT_EQ(*(begin++), entt::entity{2});
    ASSERT_EQ(*(begin++), entt::entity{1});
    ASSERT_EQ(*(begin++), entt::entity{6});
    ASSERT_EQ(begin, end);
}

TEST(SparseSet, CanModifyDuringIteration) {
    entt::sparse_set set;
    set.emplace(entt::entity{0});

    ASSERT_EQ(set.capacity(), 1u);

    const auto it = set.begin();
    set.reserve(2u);

    ASSERT_EQ(set.capacity(), 2u);

    // this should crash with asan enabled if we break the constraint
    const auto entity = *it;
    (void)entity;
}

TEST(SparseSet, ThrowingAllocator) {
    entt::basic_sparse_set<entt::entity, test::throwing_allocator<entt::entity>> set{};

    test::throwing_allocator<entt::entity>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(set.reserve(1u), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_EQ(set.capacity(), 0u);
    ASSERT_EQ(set.extent(), 0u);

    test::throwing_allocator<entt::entity>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(set.emplace(entt::entity{0}), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_EQ(set.capacity(), 0u);
    ASSERT_EQ(set.extent(), 0u);

    set.emplace(entt::entity{0});
    test::throwing_allocator<entt::entity>::trigger_on_allocate = true;

    // strong exception safety
    ASSERT_THROW(set.reserve(2u), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_EQ(set.capacity(), 1u);
    ASSERT_EQ(set.extent(), ENTT_SPARSE_PAGE);
    ASSERT_TRUE(set.contains(entt::entity{0}));

    entt::entity entities[2u]{entt::entity{1}, entt::entity{ENTT_SPARSE_PAGE}};
    test::throwing_allocator<entt::entity>::trigger_after_allocate = true;

    // basic exception safety
    ASSERT_THROW(set.insert(std::begin(entities), std::end(entities)), test::throwing_allocator<entt::entity>::exception_type);
    ASSERT_EQ(set.capacity(), 3u);
    ASSERT_EQ(set.size(), 2u);
    ASSERT_EQ(set.extent(), 2 * ENTT_SPARSE_PAGE);
    ASSERT_TRUE(set.contains(entt::entity{0}));
    ASSERT_TRUE(set.contains(entt::entity{1}));
    ASSERT_FALSE(set.contains(entt::entity{ENTT_SPARSE_PAGE}));

    set.emplace(entities[1u]);

    ASSERT_TRUE(set.contains(entt::entity{ENTT_SPARSE_PAGE}));

    // unnecessary but they test a bit of template machinery :)
    set.clear();
    set.shrink_to_fit();
    set = decltype(set){};
}
