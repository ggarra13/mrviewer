#!/usr/bin/env ruby

require 'fileutils'

EXCLUDE = %w(
libGL\.so
libm\.so
libc\.so
libX.*
.*nvidia.*libGL.so.1
libGLU.so.1
libICE.so.6
libSM.so.6
libX11.so.6
libXau.so.6
libXdmcp.so.6
libXext.so.6
libXrandr.so.2
libXrender.so.1
libasound.so.2
libc.so.6
libdl.so.2
libexpat.so.1
libfontconfig.so.1
libm.so.6
libnvidia-glcore.so.310.32
libnvidia-tls.so.310.32
libpng12.so.0
libpthread.so.0
librt.so.1
libutil.so.1
libuuid.so.1
libxcb.so.1
libz.so.1
)
EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

begin
  FileUtils.rm( Dir.glob("Release/lib/*.so*") )
rescue => e
  puts e
  exit 1
end

def process_ldd( output )
  
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

    puts "#{lib} - #{loc}"
    #FileUtils.ln_s(loc, "Release/lib/#{lib}" )
    FileUtils.cp(loc, "Release/lib/#{lib}" )
  end
end


output=`ldd Release/bin/mrViewer`
process_ldd( output )

output=`ldd Release/bin/ffmpeg`
process_ldd( output )
