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
/**
 * @file   Sequence.cpp
 * @author gga
 * @date   Sat Jul 21 04:03:15 2007
 *
 * @brief
 *
 *
 */
#include <opentimelineio/clip.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/deserialization.h>
#include <opentimelineio/effect.h>
#include <opentimelineio/freezeFrame.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/linearTimeWarp.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>
#include "opentimelineio/externalReference.h"
#include <opentimelineio/stackAlgorithm.h>

namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;
namespace otime = opentime::OPENTIME_VERSION;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "gui/mrvImageBrowser.h"
#include "gui/mrvIO.h"

#include "core/mrvBlackImage.h"
#include "core/mrvOTIO.h"
#include "mrvPreferencesUI.h"
#include "mrViewer.h"

namespace {
const char* kModule = "otio";
}

namespace mrv
{

    struct Item
    {
        const otio::Track* track = nullptr;
        const otio::Item* item = nullptr;
        otime::TimeRange range;
    };
    typedef std::vector<Item> ItemList;
    ItemList items;

    void init_items( ItemList& items,
                    const otio::SerializableObject::Retainer<otio::Timeline>& timeline )
    {
        for (const auto& j : timeline->tracks()->children())
        {
            if (auto otioTrack = dynamic_cast<const otio::Track*>(j.value))
            {
                for (const auto& k : otioTrack->children())
                {
                    if (auto otioItem = dynamic_cast<const otio::Item*>(k.value))
                    {
                        otio::ErrorStatus errorStatus;
                        const auto rangeOpt = otioItem->trimmed_range_in_parent(&errorStatus);
                        if (rangeOpt.has_value())
                        {
                            Item item;
                            item.track = otioTrack;
                            item.item = otioItem;
                            item.range = rangeOpt.value();
                            items.emplace_back(item);
                        }
                    }
                }
            }
        }
    }

bool parse_timeline(LoadList& sequences, TransitionList& transitions,
                    const otio::SerializableObject::Retainer<otio::Timeline>& timeline )
{

    otio::ErrorStatus errorStatus;
    auto video_tracks = timeline.value->video_tracks();
    auto onetrack = otio::flatten_stack(video_tracks, &errorStatus);
    if (!onetrack)
    {
        LOG_ERROR( _("Could not flatten tracks. Error: ") << errorStatus.full_description);
        return false;
    }

    std::string name;
    std::stringstream ss(name);
    ss << timeline.value->name() << " Flattened";
    auto newtimeline = otio::SerializableObject::Retainer<otio::Timeline>(new otio::Timeline(ss.str()));
    auto stack = otio::SerializableObject::Retainer<otio::Stack>(new otio::Stack());
    newtimeline.value->set_tracks(stack);
    if (!stack.value->append_child(onetrack, &errorStatus))
    {
      LOG_ERROR(_("Could not append child to stack. Error: "));
      return false;
    }

    ItemList items;
    init_items( items, newtimeline );

    for (const auto i : newtimeline.value->tracks()->children())
    {
        if (auto track = dynamic_cast<otio::Track*>(i.value))
        {
            for (auto child : track->children())
            {
                if (auto item = dynamic_cast<otio::Item*>(child.value))
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(item))
                    {
                        auto e = dynamic_cast<otio::ExternalReference*>( clip->media_reference() );
                        if ( e )
                        {

                            // auto s = clip->available_range(&errorStatus).start_time();
                            // auto d = clip->available_range(&errorStatus).duration();
                            // auto s = clip->visible_range(&errorStatus).start_time();
                            // auto d = clip->visible_range(&errorStatus).duration();
                            auto s = clip->trimmed_range(&errorStatus).start_time();
                            auto d = clip->trimmed_range(&errorStatus).duration();
                            // auto s = clip->trimmed_range_in_parent(&errorStatus)->start_time();
                            // auto d = clip->trimmed_range_in_parent(&errorStatus)->duration();
                            int64_t start = s.value();
                            int64_t duration = d.value();
                            int64_t end = start + duration - 1;
                            TRACE2( clip->name()
                                    << " start = " << start << " duration= "
                                    << duration << " end= " << end );
                            assert( end >= start );
                            LoadInfo info( e->target_url(), start, end,
                                           start, end, d.rate() );
                            sequences.push_back( info );
                        }
                        else if (auto seq = dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                        {
                            int64_t start = seq->start_frame();
                            int64_t end = seq->end_frame();
                            double fps = seq->rate();

                            // Create name of file with url_base + prefix + padding + suffix
                            std::string file = seq->target_url_base();
                            file += seq->name_prefix();

                            char buf[12];
                            int digits = seq->frame_zero_padding();
                            const char* pr = PRId64;
                            if ( digits < 10 ) pr = "d";
                            sprintf( buf, "%%0%d%s", digits, pr );

                            file += buf;
                            file += seq->name_suffix();



                            auto s = clip->trimmed_range(&errorStatus).start_time();
                            auto d = clip->trimmed_range(&errorStatus).duration();
                            int64_t first = s.value();
                            int64_t duration = d.value();
                            int64_t last  = start + duration - 1;
                            TRACE2( file << " start = " << start << " end= " << end );
                            assert( last >= first );
                            assert( end >= start );
                            LoadInfo info( file, start, end, first, last, fps );
                            sequences.push_back( info );
                        }
                    }
                    // See the documentation to understand the difference
                    // between each of these ranges:
                    // https://opentimelineio.readthedocs.io/en/latest/tutorials/time-ranges.html
                    // summarize_range("  Trimmed Range", clip->trimmed_range(&errorStatus), errorStatus);
                    // summarize_range("  Visible Range", clip->visible_range(&errorStatus), errorStatus);
                    // summarize_range("Available Range", clip->available_range(&errorStatus), errorStatus);
                    else if (auto gap = dynamic_cast<otio::Gap*>(item))
                    {
                        // auto s = gap->visible_range(&errorStatus).start_time();
                        // auto d = gap->visible_range(&errorStatus).duration();
                        auto s = gap->trimmed_range(&errorStatus).start_time();
                        auto d = gap->trimmed_range(&errorStatus).duration();
                        int64_t start = s.value();
                        int64_t duration = d.value();
                        int64_t end = start + duration - 1;
                        assert( end >= start );
                        TRACE2( "GAP start = " << start << " duration= "
                                << duration << " end= " << end );
                        LoadInfo info( _("Black Gap"), start, end, start, end,
                                       d.rate() );
                        sequences.push_back( info );
                    }
                }
            }
        }
    }

    int64_t start = AV_NOPTS_VALUE, end = AV_NOPTS_VALUE;
    for ( const auto& i : items )
    {
        const auto neighbors = i.track->neighbors_of(i.item, &errorStatus);
        TRACE( "CLIP " << i.item->name() );
        if (auto transition =
            dynamic_cast<otio::Transition*>(neighbors.first.value))
        {
            int64_t s = i.range.start_time().value();
            int64_t e = i.range.end_time_inclusive().value();
            int64_t in_off = transition->in_offset().value();
            int64_t out_off = transition->out_offset().value();
            std::string name = transition->name();


            TRACE( "TRANSITION " << name << " first s= " << s );
            // TRACE( "TRANSITION first e= " << e );
            // TRACE( "TRANSITION first in_off= " << in_off );
            TRACE( "TRANSITION " << name << " first out_off= " << out_off );
            end = s + out_off;
            TRACE( "TRANSITION " << name << " first end= " << start );
        }
        if ( start != AV_NOPTS_VALUE && end != AV_NOPTS_VALUE )
        {
            TRACE2( "DISSOLVE start= " << start << " end= " << end );
            Transition t( Transition::kDissolve, start, end );
            transitions.push_back( t );
            start = end = AV_NOPTS_VALUE;
        }
        if (auto transition =
            dynamic_cast<otio::Transition*>(neighbors.second.value))
        {
            int64_t s = i.range.start_time().value();
            int64_t e = i.range.end_time_inclusive().value();
            int64_t in_off = transition->in_offset().value();
            int64_t out_off = transition->out_offset().value();
            std::string name = transition->name();

            // TRACE( "TRANSITION second s= " << s );
            TRACE( "TRANSITION " << name << " second e= " << e );
            TRACE( "TRANSITION " << name << " second in_off= " << in_off );
            // TRACE( "TRANSITION second out_off= " << out_off );
            start = e - in_off + 1;
            TRACE( "TRANSITION " << name << " second start= " << start );

        }
        // @todo: handle other transition types
        if ( start != AV_NOPTS_VALUE && end != AV_NOPTS_VALUE )
        {
            TRACE2( "DISSOLVE start= " << start << " end= " << end );
            Transition t( Transition::kDissolve, start, end );
            transitions.push_back( t );
            start = end = AV_NOPTS_VALUE;
        }
    }

    assert( start == AV_NOPTS_VALUE && end == AV_NOPTS_VALUE );

    return true;
}

bool parse_otio( mrv::LoadList& sequences, mrv::TransitionList& transitions,
                 const char* file )
{
    otio::ErrorStatus error_status;
    otio::SerializableObject::Retainer<otio::Timeline> timeline(dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_file(file, &error_status)));

    if (!timeline)
    {
        LOG_ERROR( _("Could not open .otio file. ") );
        return false;
    }

    // Change directory to that of otio file, so that relative paths work fine.
    fs::path p = file;
    p = p.parent_path();
    int ok = chdir( p.string().c_str() );

    return parse_timeline( sequences, transitions, timeline );
}

void ImageBrowser::save_otio( mrv::Reel reel,
                              const std::string& file )
{
    otio::ErrorStatus error_status;
    auto timeline = otio::SerializableObject::Retainer<otio::Timeline>(new otio::Timeline(reel->name));
    auto track = otio::SerializableObject::Retainer<otio::Track>(new otio::Track());

    for ( unsigned i = 0 ; i < reel->images.size(); ++i )
    {
        char shotID[64];
        sprintf( shotID, "shot #%d", i );
        mrv::media& m = reel->images[i];
        CMedia* img = m->image();
        double fps = img->fps();
        otio::RationalTime s( img->start_frame(), fps );
        otio::RationalTime d( img->end_frame() - img->start_frame() + 1, fps );
        otio::TimeRange availableRange( s, d );
        otio::RationalTime start( img->first_frame(), fps );
        otio::RationalTime duration( img->last_frame() - img->first_frame() + 1,
                                     fps );
        otio::TimeRange trimmedRange( start, duration );

        otio::Item* item = NULL;
        otio::SerializableObject::Retainer<otio::MediaReference> mediaReference;

        if ( img->has_video() )
        {
            std::string path = relative_path( img->fileroot(), file );
            mediaReference = new otio::ExternalReference( path, availableRange );
            item = new otio::Clip(shotID, mediaReference, trimmedRange );
        }
        else if ( dynamic_cast< BlackImage* >( img ) )
        {
            item = new otio::Gap( trimmedRange, shotID );
        }
        else
        {
            std::string path = relative_path( img->directory(), file );
            path += "/";
            std::string name_prefix = img->name_prefix();
            int         padding     = img->frame_padding();
            std::string name_suffix = img->name_suffix();
            int frame_step = 1;
            mediaReference =
                new otio::ImageSequenceReference( path, name_prefix,
                                                  name_suffix,
                                                  img->start_frame(),
                                                  frame_step,
                                                  img->fps(),
                                                  padding,
                                                  otio::ImageSequenceReference::MissingFramePolicy::error,
                                                  availableRange );
            item = new otio::Clip(shotID, mediaReference, trimmedRange );
        }
        auto clip = otio::SerializableObject::Retainer<otio::Item>( item );
        if ( ! track.value->append_child( clip, &error_status ) )
        {
            LOG_ERROR( _("Could not append one clip to track: ")
                       << error_status.full_description );
        }
    }

    auto stack = otio::SerializableObject::Retainer<otio::Stack>(new otio::Stack());
    timeline.value->set_tracks(stack);
    if (!stack.value->append_child(track, &error_status))
    {
        LOG_ERROR( _("Could not append one track to stack: ")
                   << error_status.full_description  );
        return;
    }

    if (!timeline.value->to_json_file(file.c_str(), &error_status))
    {
        LOG_ERROR( _("Could not save .otio timeline: ")
                   << error_status.full_description );
        return;
    }

    char buf[1024];
    sprintf( buf, _("Otio timeline saved to '%s'."), file.c_str() );
    mrv::alert( buf );
}

} // namespace mrv
