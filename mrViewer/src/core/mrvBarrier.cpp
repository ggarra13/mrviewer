/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Copyright (C) 2002-2003
// David Moore, William E. Kempf
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/thread/detail/config.hpp>
#include "core/mrvBarrier.h"
#include <string> // see http://article.gmane.org/gmane.comp.lib.boost.devel/106981
#include <iostream>

namespace mrv {

  barrier::barrier(unsigned int count)
    : m_threshold(count), m_count(count), m_generation(0)
  {
    if (count == 0)
      throw std::invalid_argument("count cannot be zero.");
  }

  barrier::~barrier()
  {
  }

  void barrier::notify_all()
  {
    boost::mutex::scoped_lock lock(m_mutex);
    ++m_generation;
    m_cond.notify_all(); // sigh, all for this
  }

  void barrier::count( unsigned int c )
  {
    if      (c == 0) throw std::invalid_argument("count cannot be zero.");

    boost::mutex::scoped_lock lock(m_mutex);

    if ( c == m_threshold ) return;

    unsigned int used = m_threshold - m_count;
    if ( used >= c ) m_count = 1;
    else             m_count = c - used;

    m_threshold = c;

  }

  bool barrier::wait()
  {
    boost::mutex::scoped_lock lock(m_mutex);
    unsigned int gen = m_generation;

    if (--m_count == 0)
      {
        ++m_generation;
        m_count = m_threshold;
        m_cond.notify_all();
        return true;
      }

    while (gen == m_generation)
      m_cond.wait(lock);
    return false;
  }

} // namespace mrv
