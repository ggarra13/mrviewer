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
      FileUtils.rm("#{@debug}/lib/#{lib}")
    rescue
    end

    puts "loc: #{loc} -> lib: #{lib}"
    orig = lib
    if File.symlink?( loc )
      lib = File.readlink( loc )
      puts "#{lib} ->->-> #{orig}"
      FileUtils.cp(loc, "#{@debug}/lib/#{lib}" )
      FileUtils.ln_s( "#{lib}", "#{@debug}/lib/#{orig}" )
    else
      FileUtils.cp(loc, "#{@debug}/lib/#{lib}" )
    end
  end
end

@debug = ARGV.shift
if not @debug
  @debug = "Release"
end

release = `uname -r`.chop!

build = "BUILD/Linux-#{release}-64/"

Dir.chdir( build )

home=ENV['HOME']+"/bin/mrViewer"
FileUtils.rm_f( home )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Release/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer" )
FileUtils.rm_f( home + '-dbg' )
FileUtils.ln_s( ENV['PWD']+'/'+build+"/Debug/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer-dbg" )

libs = Dir.glob( "#{@debug}/lib/*" )
FileUtils.rm_f( libs )

exes = Dir.glob( "#{@debug}/bin/*" )

files = []

for exe in exes
  output=`ldd #{exe}`
  puts "\t#{exe}"
  puts output
  files += output.split("\n")

end

files.sort!
files.uniq!

parse( files )

FileUtils.rm_f( "#{@debug}/shaders" )
FileUtils.rm_f( "#{@debug}/docs" )
FileUtils.rm_f( "#{@debug}/ctl" )
FileUtils.cp_r( "../../shaders", "#{@debug}/" )
FileUtils.cp_r( "../../docs", "#{@debug}/" )
FileUtils.cp_r( "../../ctl", "#{@debug}/" )

`find . -name '*fuse*' -exec rm {} \\;`
