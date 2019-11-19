#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "hash_map.hpp"

#include <vector>
#include <iostream>
#include <time.h>


using namespace std;

const double eps = 10e-7;

TEST_CASE("Allocator", "[Allocator]") {
    vector<int, fefu::allocator<int>> tmp;
    for (size_t i = 0; i < 10; i++)
        tmp.push_back(i);

    REQUIRE(tmp.size() == 10);
    for (size_t i = 0; i < 10; i++)
        REQUIRE(tmp[i] == i);

    for (size_t i = 10; i > 0; i--) {
        tmp.pop_back();
        REQUIRE(tmp.size() == i - 1);
    }
}

TEST_CASE("operator[]", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    hmap[2] = "abacaba";
    CHECK(hmap[2] == "abacaba");
    int t = 5;
    hmap[t] = "abasab";
    CHECK(hmap[t] == "abasab");
    hmap[5] = "abc";
    CHECK(hmap[t] == "abc");
    CHECK(hmap[0] == "");
}

TEST_CASE("at()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    hmap[2] = "abacaba";
    CHECK(hmap.at(2) == "abacaba");
    hmap[3] = "ab";
    CHECK(hmap.at(3) == "ab");
    hmap[2] = "abc";
    CHECK(hmap.at(2) == "abc");

    const fefu::hash_map<int, string> hmap2(hmap);
    CHECK(hmap2.at(3) == "ab");
}

TEST_CASE("bucket_count()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    CHECK(hmap.bucket_count() == 10);
}

TEST_CASE("bucket()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    hmap[4] = "abc";
    size_t t = hash<int>{}(4) % 10;
    CHECK(t == hmap.bucket(4));
    hmap[-1] = "d";
    t = hash<int>{}(-1) % 10;
    CHECK(t == hmap.bucket(-1));
}

TEST_CASE("rehash()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    hmap[4] = "abc";
    hmap[-1] = "a";
    hmap[2] = "bc";
    hmap.rehash(100);
    REQUIRE(hmap.bucket_count() == 100);
    CHECK(hmap[4] == "abc");
    CHECK(hmap[-1] == "a");
    CHECK(hmap[2] == "bc");
}

TEST_CASE("reserve()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(6);
    hmap[1] = "test";
    hmap.reserve(12);
    CHECK(hmap.bucket_count() == 30);
    CHECK(hmap[1] == "test");
    hmap[-1] = "test2";
    hmap.reserve(4);
    CHECK(hmap.bucket_count() == 10);
    CHECK(hmap[1] == "test");
    CHECK(hmap[-1] == "test2");
}

TEST_CASE("load_factor", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    CHECK(hmap.load_factor() == 0.0);
    hmap[4] = "ab";
    CHECK(abs(hmap.load_factor() - 0.1) < eps);
    CHECK(abs(hmap.max_load_factor() - 0.4) < eps);
    hmap.max_load_factor(0.6);
    CHECK(abs(hmap.max_load_factor() - 0.6) < eps);
}

TEST_CASE("auto rehash", "[hash_map]") {
    fefu::hash_map<int, string> hmap(20);
    for (int i = 0; i < 40; i++) {
        hmap[i] = "aba";
    }

    CHECK(hmap.bucket_count() >= 40);
    CHECK(hmap.size() == 40);
}

TEST_CASE("contains()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    CHECK(!hmap.contains(10));
    hmap[10] = "ab";
    CHECK(hmap.contains(10));
    hmap[-1] = "bc";
    CHECK(hmap.contains(-1));
}

TEST_CASE("operator==", "[hash_map]") {
    fefu::hash_map<int, string> hmap1(10);
    fefu::hash_map<int, string> hmap2(20);
    fefu::hash_map<int, string> hmap3(20);

    

    CHECK(hmap1 == hmap2);
    CHECK(hmap2 == hmap3);

    hmap1[4] = "ab";
    hmap1[1] = "bc";

    hmap2[1] = "ab";
    hmap2[4] = "bc";
    CHECK(!(hmap1 == hmap2));
    CHECK(!(hmap1 == hmap3));

    hmap3[1] = "ab";
    hmap3[4] = "bc";
    CHECK(hmap3 == hmap2);
    CHECK(!(hmap3 == hmap1));
}

TEST_CASE("count()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    CHECK(hmap.count(2) == 0);
    hmap[2] = "ab";
    hmap[1] = "d";
    CHECK(hmap.count(1) == 1);
    CHECK(hmap.count(2) == 1);
}

TEST_CASE("hash_function()", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    auto h = hmap.hash_function();
    auto defaultHash = hash<int>{};
    CHECK(h(4) == defaultHash(4));
    CHECK(h(-1) == defaultHash(-1));
    CHECK(h(INT_MAX) == defaultHash(INT_MAX));

    fefu::hash_map<string, string> hmap2(10);
    auto h2 = hmap2.hash_function();
    auto defaultHash2 = hash<string>{};
    CHECK(h2("") == defaultHash2(""));
    CHECK(h2("abacaba") == defaultHash2("abacaba"));
}

TEST_CASE("key_eq", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    auto k = hmap.key_eq();
    CHECK(!k(1, 2));
    CHECK(k(0, 0));
    CHECK(k(5, 5));

    fefu::hash_map<string, string> hmap2(10);
    auto k2 = hmap2.key_eq();
    CHECK(!k2("aba", "aca"));
    CHECK(k2("", ""));
    CHECK(k2("test", "test"));
}

TEST_CASE("InputIterator constructor", "[hash_map]") {
    vector<pair<int, string>> data = { pair<int, string>(1, "aba"),
                                        pair<int, string>(2, "caba"),
                                        pair<int, string>(1, "caba"),
                                        pair<int, string>(2, "aba"),
                                        pair<int, string>(1, "aba"),
                                        pair<int, string>(3, "test") };

    fefu::hash_map<int, string> hmap(data.begin(), data.end());
    
    REQUIRE(hmap.size() == data.size() - 3);
    for (size_t i = 3; i < data.size(); i++) {
        REQUIRE(hmap.contains(data[i].first));
        CHECK(hmap[data[i].first] == data[i].second);
    }
}


TEST_CASE("allocator constructor, get_allocator()", "[hash_map]") {
    fefu::allocator<pair<const int, string>> t;
    fefu::hash_map<int, string> hmap(t);
    auto alloc = hmap.get_allocator();

    CHECK(typeid(t).name() == typeid(alloc).name());
}

TEST_CASE("Move constructor", "[hash_map]") {
    fefu::allocator<pair<const int, string>> t;

    fefu::hash_map<int, string> hmap1(10);
    hmap1[4] = "abc";
    fefu::hash_map<int, string> hmap2(std::move(hmap1));

    CHECK(hmap2.bucket_count() == 10);
    CHECK(hmap2[4] == "abc");

    fefu::hash_map<int, string> hmap3(10);
    hmap3[4] = "abc";
    fefu::hash_map<int, string> hmap4(std::move(hmap3), t);
    CHECK(hmap4.bucket_count() == 10);
    /*CHECK(hmap4[4] == "abc");*/
}

TEST_CASE("Copy constructor", "[hash_map]") {
    fefu::allocator<pair<const int, string>> t;

    fefu::hash_map<int, string> hmap1(10);
    hmap1[4] = "abc";
    fefu::hash_map<int, string> hmap2(hmap1);

    CHECK(hmap2.bucket_count() == 10);
    CHECK(hmap2[4] == "abc");

    fefu::hash_map<int, string> hmap3(hmap1, t);
    CHECK(hmap3.bucket_count() == 10);
    CHECK(hmap3[4] == "abc");
}

TEST_CASE("Init list constructor", "[hash_map]") {
    fefu::hash_map<int, string> hmap = { pair<int, string>(1, "aba"),
                                        pair<int, string>(2, "caba"),
                                        pair<int, string>(1, "caba"),
                                        pair<int, string>(2, "aba"),
                                        pair<int, string>(1, "aba"),
                                        pair<int, string>(3, "test") };

    REQUIRE(hmap.size() == 3);
    REQUIRE(hmap.contains(1));
    REQUIRE(hmap.contains(2));
    REQUIRE(hmap.contains(3));
    CHECK(hmap[1] == "aba");
    CHECK(hmap[2] == "aba");
    CHECK(hmap[3] == "test");
}

TEST_CASE("Assignment operators", "[hash_map]") {
    fefu::hash_map<int, string> hmap1(10);
    fefu::hash_map<int, string> hmap2(20);
    hmap2 = hmap1;
    CHECK(hmap1 == hmap2);

    fefu::hash_map<int, string> hmap3(30);
    hmap3 = std::move(hmap1);
    CHECK(hmap3 == hmap2);

    hmap2 = { pair<int, string>(1, "aba"),
                pair<int, string>(2, "caba"),
                pair<int, string>(1, "caba"),
                pair<int, string>(2, "aba"),
                pair<int, string>(1, "aba"),
                pair<int, string>(3, "test") };
    REQUIRE(hmap2.size() == 3);
    REQUIRE(hmap2.contains(1));
    REQUIRE(hmap2.contains(2));
    REQUIRE(hmap2.contains(3));
    CHECK(hmap2[1] == "aba");
    CHECK(hmap2[2] == "aba");
    CHECK(hmap2[3] == "test");
}

TEST_CASE("Size", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);

    CHECK(hmap.empty());

    hmap[1] = "aba";
    hmap[2] = "caba";
    hmap[3] = "test";

    CHECK(!hmap.empty());
    CHECK(hmap.size() == 3);
    CHECK(hmap.max_size() == MAXSIZE_T);
}


TEST_CASE("Non-const iterators", "[hash_map_iterator]") {
    fefu::hash_map<int, string> hmap(20);

    CHECK(hmap.begin() == hmap.end());

    hmap[1] = "a";
    hmap[-1] = "b";
    hmap[3] = "c";
    hmap[6] = "d";
    fefu::hash_map_iterator<std::pair<const int, string>> it = hmap.begin();
    auto tmp20 = *it;

    fefu::hash_map_const_iterator<std::pair<const int, string>> constIt(it);
    CHECK(*it == *constIt);

    CHECK(hmap[it->first] == it->second);

    fefu::hash_map_iterator<std::pair<const int, string>> tmp = it;
    auto tmp2 = it++;
    CHECK(tmp2 == tmp);
    CHECK(tmp != it);

    CHECK(hmap[(*it).first] == (*it).second);
    ++it;
    CHECK(hmap[it->first] == it->second);
    auto tmp0 = hmap[it->first];
    tmp = it++;
    CHECK(hmap[tmp->first] == tmp0);
    CHECK(++it == hmap.end());
    CHECK_THROWS(++it);
}

TEST_CASE("Const iterators", "[hash_map_iterator]") {
    const fefu::hash_map<int, string> emptyHmap;
    CHECK(emptyHmap.begin() == emptyHmap.end());
    CHECK(emptyHmap.cbegin() == emptyHmap.cend());

    fefu::hash_map<int, string> hmap0(20);
    hmap0[1] = "a";
    hmap0[-1] = "b";
    hmap0[3] = "c";
    hmap0[6] = "d";
    const fefu::hash_map<int, string> hmap(hmap0);
    auto it = hmap.begin();
    CHECK(it == hmap.cbegin());
    CHECK(hmap.at(it->first) == it->second);

    auto tmp = it;
    auto tmp2 = it++;
    CHECK(tmp2 == tmp);
    CHECK(tmp != it);

    CHECK(hmap.at((*it).first) == (*it).second);
    ++it;
    CHECK(hmap.at(it->first) == it->second);
    auto tmp0 = hmap.at(it->first);
    auto tmp3 = it++;
    CHECK(hmap.at(tmp3->first) == tmp0);
    CHECK(++it == hmap.end());
    CHECK_THROWS(++it);
}

TEST_CASE("erase", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    hmap[1] = "a";
    hmap[2] = "b";
    auto val = hmap.begin()->first;
    hmap.erase(hmap.cbegin());
    CHECK(hmap.size() == 1);
    CHECK(!hmap.contains(val));

    hmap[4] = "d";
    size_t count = hmap.erase(4);
    CHECK(count == 1);
    CHECK(!hmap.contains(4));

    count = hmap.erase(5);
    CHECK(count == 0);
    
    for (int i = 0; i < 5; i++) {
        hmap[i] = "test";
    }

    fefu::hash_map<int, string> hmapCopy(hmap);

    auto it = hmap.erase(hmap.begin(), hmap.end());
    REQUIRE(hmap.size() == 0);
    CHECK(it == hmap.end());

    hmapCopy.clear();
    CHECK(hmap.size() == 0);
}

TEST_CASE("find", "[hash_map]") {
    fefu::hash_map<int, string> hmap;
    hmap[1] = "a";
    hmap[2] = "b";
    hmap[-1] = "c";

    CHECK(hmap.find(3) == hmap.end());
    CHECK(hmap.find(2)->second == "b");

    const fefu::hash_map<int, string> constHmap(hmap);
    CHECK(constHmap.find(3) == constHmap.end());
    CHECK(constHmap.find(1)->second == "a");
}

TEST_CASE("insert", "[hash_map]") {
    fefu::hash_map<int, string> hmap;
    auto it = hmap.insert(make_pair(0, "abaca"));
    CHECK(hmap.contains(0));
    CHECK(hmap.at(0) == "abaca");
    CHECK(it.first != hmap.end());
    CHECK(it.second);

    it = hmap.insert(make_pair(0, "cabada"));
    CHECK(hmap.at(0) == "abaca");
    CHECK(!it.second);

    pair<const int, string> constPair(1, "test");
    it = hmap.insert(constPair);
    CHECK(hmap.contains(1));
    CHECK(hmap.at(1) == "test");
    CHECK(it.first != hmap.end());
    CHECK(it.second);

    pair<const int, string> constPair2(1, "null");
    it = hmap.insert(constPair2);
    CHECK(hmap.at(1) == "test");
    CHECK(!it.second);
}

TEST_CASE("insert range", "[hash_map]") {
    fefu::hash_map<int, string> hmap;
    hmap.insert(make_pair(0, "abaca"));
    hmap.insert(make_pair(1, "test"));
    fefu::hash_map<int, string> hmap2(hmap);

    vector<pair<int, string>> inputRange = { { 0, "test0" }, { 1, "test1" }, { 2, "test2"}, { 3, "test3" } };
    hmap.insert(inputRange.begin(), inputRange.end());
    CHECK(hmap.size() == 4);
    CHECK(hmap.at(0) == "abaca");
    CHECK(hmap.at(1) == "test");
    CHECK(hmap.at(2) == "test2");
    CHECK(hmap.at(3) == "test3");

    hmap2.insert({ { 0, "test0" }, { 1, "test1" }, { 2, "test2"}, { 3, "test3" } });
    CHECK(hmap2.size() == 4);
    CHECK(hmap2.at(0) == "abaca");
    CHECK(hmap2.at(1) == "test");
    CHECK(hmap2.at(2) == "test2");
    CHECK(hmap2.at(3) == "test3");
}

TEST_CASE("insert_or_assign", "[hash_map]") {
    fefu::hash_map<int, string> hmap;
    hmap.insert(make_pair(0, "abaca"));
    hmap.insert(make_pair(1, "test"));

    auto res = hmap.insert_or_assign(0, "testb");
    CHECK(hmap.at(0) == "testb");
    CHECK(!res.second);

    res = hmap.insert_or_assign(2, "testc");
    CHECK(hmap.at(2) == "testc");
    CHECK(res.second);

    const int a = 2;
    res = hmap.insert_or_assign(a, "constTest");
    CHECK(hmap.at(2) == "constTest");
    CHECK(!res.second);
}

TEST_CASE("merge", "[hash_map]") {
    fefu::hash_map<int, string> hmap1 = { { 0, "test0" },  {1, "test1" } };
    fefu::hash_map<int, string> hmap2 = { { 0, "test0" }, { 2, "test2" } };
    hmap1.merge(hmap2);
    REQUIRE(hmap1.size() == 3);
    REQUIRE(hmap2.size() == 1);

    CHECK(hmap1.at(0) == "test0");
    CHECK(hmap1.at(1) == "test1");
    CHECK(hmap1.at(2) == "test2");

    CHECK(hmap2.at(0) == "test0");

    fefu::hash_map<int, string> hmap3 = { { 1, "test1" }, { 3, "test3" }, { 4, "test4" } };
    hmap1.merge(std::move(hmap3));
    REQUIRE(hmap1.size() == 5);
    CHECK(hmap1.at(0) == "test0");
    CHECK(hmap1.at(1) == "test1");
    CHECK(hmap1.at(2) == "test2");
    CHECK(hmap1.at(3) == "test3");
    CHECK(hmap1.at(4) == "test4");
}

TEST_CASE("emplace", "hash_map") {
    fefu::hash_map<int, vector<string>> hmap;
    int k = 4;
    auto res = hmap.try_emplace(k, "aba", "caba");
    CHECK(res.second);
    CHECK(hmap.size() == 1);
    CHECK(hmap.at(4) == vector<string>({ "aba", "caba" }));

    res = hmap.try_emplace(4, "cab");
    CHECK(!res.second);
    CHECK(hmap.size() == 1);
    CHECK(hmap.at(4) == vector<string>({ "aba", "caba" }));

    vector<string> vec = { "cab", "dab" };
    res = hmap.emplace(4, vec);
    CHECK(!res.second);
    CHECK(hmap.size() == 1);
    CHECK(hmap.at(4) == vector<string>({ "aba", "caba" }));

    res = hmap.emplace(1, vec);
    CHECK(res.second);
    CHECK(hmap.size() == 2);
    CHECK(hmap.at(1) == vec);
}

// ===========================================
//              Exceptions
// ===========================================

TEST_CASE("exceptions", "[hash_map]") {
    fefu::hash_map<int, string> hmap(10);
    const fefu::hash_map<int, string> hmap2(hmap);
    CHECK_THROWS(hmap.at(2));
    CHECK_THROWS(hmap2.at(2));
    CHECK_THROWS(hmap.bucket(2));
    CHECK_THROWS(hmap.max_load_factor(1.2));
    CHECK_THROWS(hmap.max_load_factor(-0.4));

    fefu::hash_map_iterator<pair<int, string>> iter;
    fefu::hash_map_const_iterator<pair<int, string>> constIter;
    CHECK_THROWS(*iter);
    CHECK_THROWS(*constIter);

    CHECK_THROWS(hmap.erase(hmap.cend()));
    CHECK_THROWS(hmap.erase(hmap.begin(), hmap.end()));
}


#ifdef BENCHMARK

// ===========================================
//              Benchmark
// ===========================================

void benchmark_t1(size_t rounds) {
    fefu::hash_map<int, string> hmap(10);
    clock_t start = clock();

    for (size_t i = 0; i < rounds; i++) {
        hmap[i] = "test";
    }
    CHECK(hmap.bucket_count() >= rounds);
    CHECK(hmap.size() == rounds);

    double time = ((double)clock() - start) / CLOCKS_PER_SEC;

    printf("BENCHMARK: operator[]\n");
    printf(" - rounds: %d\n", rounds);
    printf(" - time taken: %.2fs\n", time);
}

TEST_CASE("BENCHMARK1", "[Benchmark]") {
    size_t rounds = 10000;
    benchmark_t1(rounds);
}

TEST_CASE("BENCHMARK2", "[Benchmark]") {
    size_t rounds = 100000;
    benchmark_t1(rounds);
}

TEST_CASE("BENCHMARK3", "[Benchmark]") {
    size_t rounds = 1000000;
    benchmark_t1(rounds);
}

#endif // BENCHMARK