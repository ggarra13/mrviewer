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

build = "BUILD/Linux-#{release}-64/"

Dir.chdir( build )

home=ENV['HOME']+"/bin/mrViewer" 
FileUtils.rm_f( home )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Release/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer" )
FileUtils.rm_f( home + '-dbg' )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Debug/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer-dbg" )

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
  FileUtils.cp(loc, "Release/lib/#{lib}" )

end

FileUtils.rm_f( "Release/shaders" )
FileUtils.rm_f( "Release/docs" )
FileUtils.rm_f( "Release/ctl" )
FileUtils.rm_f( "Release/HISTORY.txt" )
FileUtils.rm_f( "Release/LICENSE.txt" )
FileUtils.rm_f( "Release/Videos.txt" )
FileUtils.cp_r( "../../shaders", "Release/" )
FileUtils.cp_r( "../../docs", "Release/" )
FileUtils.cp_r( "../../ctl", "Release/" )
FileUtils.cp( "../../HISTORY.txt", "Release/" )
FileUtils.cp( "../../LICENSE.txt", "Release/" )
FileUtils.cp( "../../Videos.txt", "Release/" )
