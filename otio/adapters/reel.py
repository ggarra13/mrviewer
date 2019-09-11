#!/usr/bin/python3.6
#
# Copyright 2019 Film Aura, LLC. - Gonzalo Garramuno
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

"""OpenTimelineIO mrViewer's Reel Adapter.

This adapter requires the use of the stand-alone ffprobe utility that comes
with ffmpeg.  It should be in the user's PATH.

"""
import os
import sys
import re
import copy
import subprocess
from fractions import Fraction
from datetime import date, datetime

try:
    from pathlib import Path
except ImportError:
    pass
try:
    from urllib import unquote
except ImportError:
    from urllib.parse import unquote

import opentimelineio as otio

META_NAMESPACE = "reel"

class ShotDetectError(Exception):
    pass

class FFProbeFailedError(ShotDetectError):
    pass


def common_path(directories):
    norm_paths = [os.path.abspath(p) + os.path.sep for p in directories]
    if sys.version_info > (3,5):
        return os.path.dirname(os.path.commonpath(norm_paths))
    else:
        return os.path.dirname(os.path.commonprefix(norm_paths))


def _ffprobe_audio(name, full_path, dryrun=False):
    """Parse the audio from the file using ffprobe."""

    _, err = _ffprobe_output(
    name,
    full_path,
    dryrun,
    arguments=["{0}".format(full_path)],
    message="audio"
    )


    if dryrun:
        return 1, 1

    length = 0
    has_audio = 0

    r = re.compile( r'\s*Duration:\s+(\d+)\:(\d+)\:(\d+)\.(\d+).*' )
    for line in err.split('\n'):
        obj = re.match(r, line)
        if obj:
            hour = obj.group(1)
            mins = obj.group(2)
            secs = obj.group(3)
            ms   = obj.group(4)
            length = float(hour) * 3600 + float(mins) * 60 + float(secs) \
              + float(ms) * 0.001

        if not "Stream" in line:
            continue

        if "Audio" in line:
            has_audio += 1

    return has_audio, length

def _ffprobe_video_audio(name, full_path, dryrun=False):
    """Parse the video/audio from the file using ffprobe."""

    _, err = _ffprobe_output(
    name,
    full_path,
    dryrun,
    arguments=["{0}".format(full_path)],
    message="video/audio"
    )

    if dryrun:
        return 1, 1

    has_video = 0
    has_audio = 0

    for line in err.split('\n'):
        if not "Stream" in line:
            continue

        if "Video" in line:
            has_video += 1

        if "Audio" in line:
            has_audio += 1

    return has_video, has_audio

def _ffprobe_fps(name, full_path, dryrun=False):
    """Parse the framerate from the file using ffprobe."""

    _, err = _ffprobe_output(
    name,
    full_path,
    dryrun,
    arguments=["{0}".format(full_path)],
    message="framerate"
    )

    if dryrun:
        return 1.0

    for line in err.split('\n'):
        if not ("Stream" in line and "Video" in line):
            continue

    bits = line.split(",")
    for bit in bits:
        if "fps" not in bit:
            continue
        fps = float([b for b in bit.split(" ") if b][0])
        return fps

    # fallback FPS is just 30.0 fps for audio
    print("No FPS found in",name,". Defaulting to 30.0")
    return 30.0


def _ffprobe_output(
    name,
    full_path,
    dryrun=False,
    arguments=None,
    message="shot breaks"
):
    """ Run ffprobe and return resulting output """

    arguments = arguments or [
    "-show_frames",
    "-of",
    "compact=p=0",
    "-f",
    "lavfi",
    "movie={0},select=gt(scene\\,.1)".format(full_path)
    ]

    if message:
        print("Scanning {0} for {1}...".format(name, message))

    cmd = ["ffprobe"] + arguments

    if dryrun:
        print(" ".join(cmd))
        return ("", "")

    proc = subprocess.Popen(
    cmd,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if proc.returncode != 0:
        raise FFProbeFailedError(
            "FFProbe Failed with error: {0}, output: {1}".format(
            err, out
            )
        )

    return out.decode(), err.decode()


def _is_special( clip ):
    if clip.schema_name() == 'Gap' or \
      clip.schema_name() == 'Transition' or \
      clip.schema_name() == 'Track' or \
      clip.schema_name() == 'Stack':
        return True
    return False


class Otio2Reel(object):
    """
    This object is responsible for knowing how to convert an otio into a
    reel file
    """


    def __init__(self, otio_timeline, filepath, rel_path = False):
        self.otio_timeline = otio_timeline
        self.relative_paths = rel_path
        self.filepath = os.path.abspath( filepath )
        self.title = """
#
# File created with {}
#
# Date: {}
#

Version 4.0
Ghosting 5 5
""".format( "otio reel adapter",
            datetime.now().strftime( '%Y-%m-%d %H:%M:%S' ))

        if self.otio_timeline.schema_name() == "Timeline":
            self.timelines = [self.otio_timeline]
        else:
            self.timelines = list(
            self.otio_timeline.each_child(
                descended_from_type=otio.schema.Timeline
                )
            )


    """
    Convert an absolute filename to a relative filename to that of the reel
    being saved.
    """
    def _relative_filename( self, filename ):
        if filename == 'Checkered':
            return filename
        if filename[0] != '/' and filename[1] != ':':
            filename = os.path.abspath( filename )
        if not self.relative_paths:
            return filename
        try:
            filename = Path( filename ).relative_to( os.path.dirname(self.filepath) )
        except ValueError:
            pass
        return filename

    def _timeline_to_reel( self, timeline ):

        if self.timeline_index != '':
            self.timeline_index += 1

        filename = re.sub( r"\.(?:otio|reel)", "", self.filepath, flags=re.I )

        f = open( filename + '{}.reel'.format(self.timeline_index), 'w' )
        f.write( self.title )

        audio_tracks = timeline.audio_tracks()
        video_tracks = timeline.video_tracks()

        video_tracks = otio.algorithms.flatten_stack(video_tracks)

        newtimeline = otio.schema.Timeline( \
            name="{} Flattened".format(timeline.name))
        newtimeline.tracks[:] = [video_tracks]

        # keep the audio track(s) as-is
        newtimeline.tracks.extend(copy.deepcopy(audio_tracks))

        timeline = newtimeline
        video_tracks = timeline.video_tracks()
        audio_tracks = timeline.audio_tracks()

        audio_start = False
        video_start = True

        if len(video_tracks) != 0 and len(video_tracks[0]):
            clip = video_tracks[0][0]
            if not _is_special(clip):
                playhead = clip.trimmed_range().start_time
            else:
                playhead = clip.trimmed_range_in_parent().start_time
        elif len(audio_tracks) != 0 and len(audio_tracks[0]):
            clip = audio_tracks[0][0]
            if not _is_special(clip):
                playhead = clip.trimmed_range().start_time
            else:
                playhead = clip.trimmed_range_in_parent().start_time
        else:
            print(timeline.name,"is empty")
        return


        if len(video_tracks) != 0 and len(video_tracks[0]):
            clip = video_tracks[0][0]
        else:
            clip = audio_tracks[0][0]
        if not _is_special(clip):
            start_time = clip.trimmed_range().start_time
        else:
            start_time = clip.trimmed_range().start_time
        playhead = start_time

        for t in range(0, len(audio_tracks) ):
            clip = audio_tracks[t][0]
            if not _is_special(clip):
                start_time = clip.trimmed_range().start_time
            else:
                start_time = clip.trimmed_range().start_time
            if start_time <= playhead:
                playhead = start_time
                audio_start = True


            duration = otio.opentime.RationalTime( 0, 24 )

        if video_start:
            for c in range(0, len(video_tracks[0]) ): # number of clips
                clip = video_tracks[0][c]
                trange = None
                if _is_special(clip):
                    trange = clip.trimmed_range_in_parent()
                else:
                    trange = clip.media_reference.available_range
                if not trange:
                    trange = clip.trimmed_range()
                start = trange.start_time
                start = int(start.value)
                end = trange.duration
                duration += end
                end = int(end.value)
                end += start
                start += 1
                first = start
                last  = end
                if clip.schema_name() != 'Transition':
                    srange = clip.source_range
                    if srange:
                        first = srange.start_time
                        first = int(first.value)
                        first += 1
                        last  = srange.duration
                        last  = first + int(last.value) - 1
                    if not _is_special( clip ) \
                      and not clip.media_reference.is_missing_reference:
                        filename = clip.media_reference.target_url
                        filename = re.sub("^file://", "", filename )
                        filename = self._relative_filename( filename )
                    elif not _is_special(clip) and \
                      clip.media_reference.is_missing_reference:
                        filename = 'Checkered'
                    else:
                        filename = 'Black Gap'
                    f.write( "\"{}\" {} {} {} {} {}\n".format( \
                        filename, first, last, start, end, \
                        trange.duration.rate ))

            for at in range( len(audio_tracks) ):
                for ct in range( len(audio_tracks[at]) ):
                    aclip = audio_tracks[at][ct]
                    aclip.name = "audio {}".format(ct)
                    atrange= aclip.trimmed_range_in_parent()
                    trange = clip.trimmed_range_in_parent()
                    try:
                        srange = aclip.trimmed_range()
                    except AttributeError:
                        pass
                    astart = srange.start_time
                    astart = int(astart.value)

                    if _is_special(aclip):
                        pass
                    elif trange.overlaps(atrange) and \
                    ( not _is_special(aclip) and \
                        not _is_special(clip) and \
                        not aclip.media_reference.is_missing_reference \
                        and not clip.media_reference.is_missing_reference \
                        and aclip.media_reference.target_url != \
                    clip.media_reference.target_url ):
                        filename = aclip.media_reference.target_url
                        filename = re.sub("^file://", "", filename )
                        filename = self._relative_filename( filename )
                        f.write( "audio: {}\n".format( filename ) )
                        f.write( "audio offset: {}\n".format(astart) )

        f.write( "EDL\n" )
        f.close


    def to_reel(self):
        """
        Convert an otio to a reel

        Returns:
        str: .reel content
        """

        self.timeline_index = ''

        if len(self.timelines) > 1:
            self.timeline_index = 0

        for timeline in self.timelines:
            self._timeline_to_reel(timeline)


class Reel2Otio(object):
    """
    This object is responsible for knowing how to convert a Reel file
    into an otio timeline
    """

    """
    Convert an absolute filename to a relative filename to that of the otio file
    being saved.  Under python2.7, this method CAN fail, as its comparison is
    done with strings.  Under python3.5, the method should work reliably.
    """
    def _relative_filename( self, filename ):
        if filename == 'Checkered':
            return filename
        if filename[0] != '/' and filename[1] != ':' \
          and not self.relative_paths:
            filename = os.path.abspath( filename )
            return filename
        try:
            filename = Path( filename ).relative_to( self.filepath )
        except ValueError:
            pass
        return filename


    def _find_audio_offset( self, line ):
        r = re.compile( r'audio offset:\s*(\d+)' )
        obj = re.match(r, line)
        if not obj:
            return False
        self.aoffset = int( obj.group(1) )

    def _find_audio_line( self, line ):
        r = re.compile( r'audio:\s*([^\n]+)' )
        obj = re.match(r, line)
        if not obj:
            return False
        self.media_path = obj.group(1)
        if self.media_path == None:
            return False

        if self.media_path[0] != '/' and self.media_path[1] != ':':
            path = self.filepath + '/' + self.media_path
        else:
            path = self.media_path

        has_audio, length = _ffprobe_audio( self.media_path, path, False )

        if has_audio:
            if len(self.audio_tracks) == 0:
                for i in range(1, has_audio + 1):
                    track = otio.schema.Track()
                    track.name = "Audio #{}".format(i)
                    track.kind = "Audio"
                    self.audio_tracks.append( track )
                    self.timeline.tracks.append(track)

        if not self.fps and self.old_fps:
            self.fps = self.old_fps

        start_time = self.playhead.rescaled_to(self.fps)
        end_time_in_seconds = length

        end_time_exclusive = otio.opentime.RationalTime(
            float(end_time_in_seconds),
            1.0
            ).rescaled_to(self.fps)
        end_time_exclusive += start_time

        if end_time_exclusive > self.end_time_exclusive:
            print("Audio", self.media_path, "is too long.  Will use a portion of it.")

        path = self._relative_filename( self.media_path )
        self.clip = otio.schema.Clip(name=self.media_path)
        self.clip.media_reference = otio.schema.ExternalReference(
            target_url="file://" + str(path) )
        self.clip.metadata[ os.environ.get('STUDIO') ] = {
            'seqID': os.environ.get('SEQ'),
            'shotID': os.environ.get('SHOT'),
            'takeID': os.environ.get('TAKE'),
            'artist': os.environ.get('USER') or os.environ.get('USERNAME')
            }
        self.clip.source_range = otio.opentime.range_from_start_end_time( \
                        start_time, self.end_time_exclusive )
        self.clip.media_reference.available_range = \
          otio.opentime.range_from_start_end_time( \
                    start_time, end_time_exclusive )

        return has_audio

    def _find_video_audio_line( self, line ):

        r = re.compile( r'^\"(.+?)\"\s+(\d+)\s+(\d+)\s*(\d+)?\s*(\d+)?\s+([-+]?(\d*[.])?\d*([eE][-+]?\d+)?)?' )
        obj = re.match(r, line)
        if not obj:
            return False, False
        self.media_path = obj.group(1)
        self.first_frame = obj.group(2)
        self.last_frame = obj.group(3)
        self.start_frame = obj.group(4)
        self.end_frame = obj.group(5)
        if obj.group(6):
            self.fps = float(obj.group(6))
        else:
            self.fps = None


        if self.media_path[0] != '/' and self.media_path[1] != ':':
            path = self.filepath + '/' + self.media_path
        else:
            path = self.media_path

        if not self.fps:
            self.fps = _ffprobe_fps( self.media_path, path, False )

        has_video, has_audio = _ffprobe_video_audio( self.media_path, path,
                                False )
        return has_video, has_audio

    def _find_video_line( self, line ):

        has_video = False
        has_audio = False
        has_gap   = False
        has_pic   = True

        r = re.compile( r"^\"(Black Gap|Checkered)\"\s+(\d+)\s+(\d+)\s+(\d+)?\s+(\d+)\s+([+-]?\d*\.?\d*)?$" )
        obj = re.match(r, line)
        if obj:
            self.media_path = obj.group(1)
            self.first_frame = obj.group(2)
            self.last_frame = obj.group(3)
            self.start_frame = obj.group(4)
            self.end_frame = obj.group(5)
            self.fps = float(obj.group(6))
            has_video = True
            if self.media_path == 'Black Gap':
                has_gap = True
            elif self.media_path == 'Checkered':
                has_pic = False
        else:
            has_video, has_audio = self._find_video_audio_line( line )


        if has_video and len(self.video_tracks) == 0:
            for i in range(1, has_video + 1 ):
                track = otio.schema.Track()
                track.name = "Video #{}".format(i)
                track.kind = "Video"
                self.video_tracks.append( track )
                self.timeline.tracks.append(track)

        if has_audio and len(self.audio_tracks) == 0:
            for i in range(1, has_audio + 1):
                track = otio.schema.Track()
                track.name = "Audio #{}".format(i)
                track.kind = "Audio"
                self.audio_tracks.append( track )
                self.timeline.tracks.append(track)

        if not self.fps and self.old_fps:
            self.fps = self.old_fps

        if not self.start_frame:
            self.start_frame = self.first_frame

        if not self.end_frame:
            self.end_frame = self.last_frame

        start_time = (float(self.first_frame) - 1.0) / self.fps
        end_time_in_seconds = float(self.last_frame) / self.fps

        start_time = otio.opentime.RationalTime(
            float(start_time),
            1.0
            ).rescaled_to(self.fps)
        end_time_exclusive = otio.opentime.RationalTime(
            float(end_time_in_seconds),
            1.0
            ).rescaled_to(self.fps)
        # print "START time:",start_time, "END time:", end_time_exclusive

        if has_gap:
            self.clip = otio.schema.Gap()
        else:
            self.clip = otio.schema.Clip()

            path = self._relative_filename( self.media_path )
            self.clip.media_reference = otio.schema.ExternalReference(
            target_url="file://" + str(path) )

        self.clip.source_range = \
          otio.opentime.range_from_start_end_time( \
                start_time, end_time_exclusive \
            )

        if not has_gap and ( self.first_frame != self.start_frame or \
                    self.last_frame != self.end_frame ):

            start_time = (float(self.start_frame) - 1.0) / self.fps
            end_time_in_seconds = float(self.end_frame) / self.fps

            start_time = otio.opentime.RationalTime(
            float(start_time),
            1.0
            ).rescaled_to(self.fps)
            end_time_exclusive = otio.opentime.RationalTime(
            float(end_time_in_seconds),
            1.0
            ).rescaled_to(self.fps)
            #  print "start time:",start_time, "end time:", end_time_exclusive

            self.clip.media_reference.available_range = \
              otio.opentime.range_from_start_end_time( \
                                                           start_time, \
                                                           end_time_exclusive \
                                                           )

            self.clip.metadata[ os.environ.get('STUDIO') ] = { \
                'seqID': os.environ.get('SEQ'), \
                'shotID': os.environ.get('SHOT'), \
                'takeID': os.environ.get('TAKE'), \
                'artist': os.environ.get('USER') or os.environ.get('USERNAME') \
            }

        self.clip.name = "shot {0}".format(self.shot_index)
        self.shot_index += 1

        self.end_time_exclusive = end_time_exclusive

        return has_video, has_audio, has_gap, has_pic



    def __init__(self, reel_text, filename, rel_path = False):
        self.timeline = otio.schema.Timeline()
        self.relative_paths = rel_path
        self.filepath = os.path.abspath( os.path.dirname( filename ) )

        lines = reel_text.split("\n")
        lines = [i.strip() for i in lines]

        self.timeline.name = filename
        self.timeline.metadata[ os.environ.get('STUDIO') ] = {
            'showID': os.environ.get('SHOW')
            }
        self.old_fps = None
        self.fps = None

        # Remove 'Version 3.0' or 'Version 3'
        r = re.compile( r"^\s*Version\s+\d+\.*\d*" )
        lines = list(filter(lambda i: not r.search(i), lines))

        r = re.compile( r"^(?:|#.*|Ghosting \d \d|EDL)$" )
        lines = list(filter(lambda i: not r.search(i), lines))

        r = re.compile( r"^(?:GL.*Shape.*)$" )
        lines = list(filter(lambda i: not r.search(i), lines))

        self.playhead = otio.opentime.RationalTime( 0, 1.0 )

        self.video_tracks = []
        self.audio_tracks = []

        self.audio_index = 1
        self.shot_index = 1

        idx = 0
        while idx < len(lines):
            self.aoffset  = 0
            has_video = has_audio = has_audio2 = 0
            if not has_video and not has_audio:
                has_video, has_audio, has_gap, has_pic = \
                  self._find_video_line( lines[idx] )

            if has_video or has_pic:
                for track in self.video_tracks:
                    track.append(copy.deepcopy(self.clip))

            if idx+1 < len(lines):
                has_audio2 = self._find_audio_line( lines[idx+1] )
                if has_audio2:
                    idx += 1


            if has_video or has_pic:
                if has_audio or ( has_audio2 and idx+1 < len(lines) ):
                    if has_audio2:
                        self.aoffset = None
                        self._find_audio_offset( lines[idx+1] )

                        trange = self.clip.trimmed_range()
                        if self.aoffset is not None:
                            idx += 1

                            start_time = \
                            otio.opentime.RationalTime( self.aoffset, \
                                        trange.duration.rate)

                            self.clip.source_range = \
                            otio.opentime.range_from_start_end_time( \
                            start_time, \
                            start_time + self.playhead + \
                            trange.duration \
                                )
                        else:
                            self.clip.source_range = trange
                    else:
                        trange = self.clip.trimmed_range()
                        self.clip = otio.schema.Gap()
                        self.clip.source_range = trange
                        if not self.audio_tracks:
                            track = otio.schema.Track()
                            track.name = "Audio 1"
                            track.kind = "Audio"
                            self.audio_tracks.append( track )


                for track in self.audio_tracks:
                    print(self.clip)
                    track.append(copy.deepcopy(self.clip))

            self.playhead = self.end_time_exclusive
            self.old_fps = self.fps
            self.fps = None
            idx += 1

    def to_otio(self):
        """
        Convert a reel to an otio timeline

        Returns:
        OpenTimeline: An OpenTimeline Timeline object
        """
        return self.timeline

# --------------------
# adapter requirements
# --------------------
    """
    Necessary read method for otio adapter

    Args:
    filepath (str): A reel filename

    Returns:
    OpenTimeline: An OpenTimeline object
    """

def read_from_file(filepath, relative_paths = False ):
    input_str = "".join( open( filepath, 'r' ).readlines() )
    return Reel2Otio(input_str, filepath, relative_paths).to_otio()

def write_to_file(input_otio, filepath, relative_paths = False ):
    """
    Necessary write method for otio adapter

    Args:
    input_otio (OpenTimeline): An OpenTimeline object

    Returns:
    str: The string contents of a Reel file
    """

    return Otio2Reel( input_otio, filepath, relative_paths ).to_reel()
