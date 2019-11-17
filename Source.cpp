#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "hash_map.hpp"

#include <vector>



using namespace std;

TEST_CASE("Allocator", "[Allocator]") {
    vector<int, fefu::allocator<int>> tmp;
    tmp.push_back(1);
    tmp.push_back(2);
    tmp.push_back(3);
    tmp.pop_back();
}
