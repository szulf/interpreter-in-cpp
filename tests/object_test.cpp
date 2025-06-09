#include <gtest/gtest.h>

#include "object.h"

TEST(object, string_hash_key) {
    using namespace interp;

    object::string hello1{"Hello World"};
    object::string hello2{"Hello World"};
    object::string diff1{"My name is johny"};
    object::string diff2{"My name is johny"};

    ASSERT_EQ(hello1.get_hash_key(), hello2.get_hash_key());
    ASSERT_EQ(diff1.get_hash_key(), diff2.get_hash_key());
    ASSERT_NE(hello1.get_hash_key(), diff1.get_hash_key());
}
