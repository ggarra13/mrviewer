#!/usr/bin/env ruby

require 'fileutils'

EXCLUDE = %w(
libGL\.so
libm\.so
libgcc_s.*
libc\.so*
libstdc\+\+.*
.*nvidia.*
)
EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

release = `uname -r`.chop!

Dir.chdir( "BUILD/Linux-#{release}-64/" )

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

FileUtils.rm_f( "Release/shaders" )
FileUtils.rm_f( "Release/docs" )
FileUtils.rm_f( "Release/ctl" )
FileUtils.rm_f( "Release/HISTORY.txt" )
FileUtils.rm_f( "Release/LICENSE.txt" )
FileUtils.rm_f( "Release/Videos.txt" )
FileUtils.ln_s( "../../../shaders", "Release/shaders" )
FileUtils.ln_s( "../../../docs", "Release/docs" )
FileUtils.ln_s( "../../../ctl", "Release/ctl" )
FileUtils.ln_s( "../../../HISTORY.txt", "Release/HISTORY.txt" )
FileUtils.ln_s( "../../../LICENSE.txt", "Release/LICENSE.txt" )
FileUtils.ln_s( "../../../Videos.txt", "Release/Videos.txt" )
