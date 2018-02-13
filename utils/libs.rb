#!/usr/bin/env ruby

require 'fileutils'

EXCLUDE = %w(
libGL\.so
libGLdispatch\.so
libGLX\.so
libX.*
.*nvidia.*
libpthread.*
libresolv.*
libm\.so
libc\.so.*
librt.*
libdl.*
libxcb.*
libasound.*
libfontconfig.*
)

EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

def parse( files )

  for line in files
    lib, loc = line.split(" => ")

    next if not loc or not lib

    loc.sub!(/\s*\(.*\)$/, '')
    lib.sub!(/^\s*/, '')

    if loc.empty?
      puts "#{lib} empty loc"
      next
    end

    if lib =~ EXCLUDE_REGEX
      puts "Exclude #{lib}"
      next
    end

    begin
      FileUtils.rm("Release/lib/#{lib}")
    rescue
    end

    puts "#{loc} -> #{lib}"
    orig = lib
    if File.symlink?( loc )
      lib = File.readlink( loc )
      puts "#{lib} ->->-> #{orig}"
      FileUtils.cp(loc, "Release/lib/#{lib}" )
      FileUtils.ln_s( "#{lib}", "Release/lib/#{orig}" )
    else
      FileUtils.cp(loc, "Release/lib/#{lib}" )
    end
  end
end

release = `uname -r`.chop!

build = "BUILD/Linux-#{release}-64/"

Dir.chdir( build )

home=ENV['HOME']+"/bin/mrViewer"
FileUtils.rm_f( home )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Release/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer" )
FileUtils.rm_f( home + '-dbg' )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Debug/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer-dbg" )

libs = Dir.glob( "Release/lib/*" )
FileUtils.rm_f( libs )

exes = Dir.glob( "Release/bin/*" )

files = []

for exe in exes
  output=`ldd #{exe}`
  files += output.split("\n")
end

files.sort!
files.uniq!


parse( files )

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

`find . -name '*fuse*' -exec rm {} \\;`
