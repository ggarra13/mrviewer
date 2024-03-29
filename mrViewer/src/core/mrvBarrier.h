/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
// Copyright (C) 2007-8 Anthony Williams
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef mrvBarrier_h
#define mrvBarrier_h

#include "boost/thread/xtime.hpp"

#include "mrvThread.h"

namespace mrv
{

class barrier
{
public:
    typedef boost::mutex               Mutex;
    typedef boost::condition_variable  Condition;

public:
    barrier(unsigned int count)
        : m_threshold(count), m_count(count), m_generation(0)
    {
        if (count == 0)
            boost::throw_exception(std::invalid_argument("count cannot be zero."));
    }

    ~barrier()
    {
        notify_all();
    }

    inline void threshold( unsigned x ) {
        if (x == 0)
            boost::throw_exception(std::invalid_argument("count cannot be zero."));
        unsigned used = m_threshold - m_count;
        m_threshold = x;
        if ( used > m_threshold ) m_count = 0;
        else m_count = m_threshold - used;
    }
    inline unsigned threshold()   {
        SCOPED_LOCK( m_mutex );
        return m_threshold;
    }
    inline unsigned generation()  {
        SCOPED_LOCK( m_mutex );
        return m_generation;
    }
    inline unsigned count()       {
        SCOPED_LOCK( m_mutex );
        return m_count;
    }
    inline unsigned used()        {
        SCOPED_LOCK( m_mutex );
        return m_threshold - m_count;
    }

    inline bool is_active()  {
        SCOPED_LOCK( m_mutex );
        return (m_count < m_threshold);
    }

    inline void notify_all()
    {
        SCOPED_LOCK( m_mutex );
        ++m_generation;
        m_count = m_threshold;
        m_cond.notify_all();
    }

    inline bool wait()
    {
        SCOPED_LOCK( m_mutex );
        unsigned int gen = m_generation;

        if (--m_count == 0)
        {
            m_generation++;
            m_count = m_threshold;
            m_cond.notify_all();
            return true;
        }

        while (gen == m_generation)
            CONDITION_WAIT( m_cond, m_mutex );

        return false;
    }

protected:
    Mutex        m_mutex;
    Condition    m_cond;
    unsigned int m_threshold;
    unsigned int m_count;
    unsigned int m_generation;
};

}   // namespace mrv

#endif
