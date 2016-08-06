//
// random_access_handle_service.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/windows/random_access_handle_service.hpp>

#include <boost/asio.hpp>
#include "../unit_test.hpp"

BOOST_ASIO_TEST_SUITE
(
  "windows/random_access_handle_service",
  BOOST_ASIO_TEST_CASE(null_test)
)
