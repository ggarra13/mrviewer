/*
 * Copyright (c) 2012-2014 Glen Joseph Fernandes 
 * glenfe at live dot com
 *
 * Distributed under the Boost Software License, 
 * Version 1.0. (See accompanying file LICENSE_1_0.txt 
 * or copy at http://boost.org/LICENSE_1_0.txt)
 */
#include <boost/detail/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared_array.hpp>

int main() {
#if !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)
    {
        boost::shared_ptr<int[][2]> a1 = boost::make_shared<int[][2]>(2, {0, 1});
        BOOST_TEST(a1[0][0] == 0);
        BOOST_TEST(a1[0][1] == 1);
        BOOST_TEST(a1[1][0] == 0);
        BOOST_TEST(a1[1][1] == 1);
    }

    {
        boost::shared_ptr<int[2][2]> a1 = boost::make_shared<int[2][2]>({ 0, 1 });
        BOOST_TEST(a1[0][0] == 0);
        BOOST_TEST(a1[0][1] == 1);
        BOOST_TEST(a1[1][0] == 0);
        BOOST_TEST(a1[1][1] == 1);
    }

    {
        boost::shared_ptr<const int[][2]> a1 = boost::make_shared<const int[][2]>(2, { 0, 1 });
        BOOST_TEST(a1[0][0] == 0);
        BOOST_TEST(a1[0][1] == 1);
        BOOST_TEST(a1[1][0] == 0);
        BOOST_TEST(a1[1][1] == 1);
    }

    {
        boost::shared_ptr<const int[2][2]> a1 = boost::make_shared<const int[2][2]>({ 0, 1 });
        BOOST_TEST(a1[0][0] == 0);
        BOOST_TEST(a1[0][1] == 1);
        BOOST_TEST(a1[1][0] == 0);
        BOOST_TEST(a1[1][1] == 1);
    }
#endif

    return boost::report_errors();
}
