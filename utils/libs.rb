#!/usr/bin/env ruby

require 'fileutils'

EXCLUDE = %w(
libGL
libc
libstdc\+\+
libm\.so
.*nvidia.*
)
EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

output=`ldd Release/bin/mrViewer`

files = output.split("\n")
files.sort!

for line in files
  lib, loc = line.split(" => ")
  
  next if not loc or not lib

  loc.sub!(/\s*\(.*\)$/, '')
  lib.sub!(/^\s*/, '')

  next if loc.empty?

  if lib =~ EXCLUDE_REGEX
    puts "Exclude #{lib}"
    next
  end

  begin
    FileUtils.rm("Release/lib/#{lib}")
  rescue
  end

  puts "#{loc} -> #{lib}"
  FileUtils.ln_s(loc, "Release/lib/#{lib}" )

end

