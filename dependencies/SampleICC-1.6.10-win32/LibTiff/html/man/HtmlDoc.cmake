# CMake documentation generation for libtiff
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

string(REPLACE "^" ";" DOCFILES "${DOCFILES}")

set(INDEXSTART "<html><head><title>Libtiff HTML manpage index</title></head><body bgcolor=white><ul><h2>Man Pages</h2><p>")
set(INDEXEND "</ul></body></html>")

set(indexcontent "${INDEXSTART}
")

foreach(doc ${DOCFILES})
  string(REGEX REPLACE "(.*)\\.html$" "\\1" man "${doc}")
  execute_process(COMMAND groff -Thtml -mandoc "${MANSRCDIR}/${man}"
    OUTPUT_FILE "${HTMLMANDIR}/${doc}"
    RESULT_VARIABLE GROFF_STATUS)
  if(GROFF_STATUS)
    message(FATAL_ERROR "Groff failed to generate HTML manpage")
  endif()
  message(STATUS "Generated ${HTMLMANDIR}/${doc} from ${MANSRCDIR}/${man}")

  set(indexcontent "${indexcontent}<li><a href=\"${doc}\">${man}</a>
")
endforeach()

set(indexcontent "${indexcontent}${INDEXEND}
")
file(WRITE "${HTMLMANDIR}/${INDEXFILE}" "${indexcontent}")
message(STATUS "Generated ${HTMLMANDIR}/${INDEXFILE}")
