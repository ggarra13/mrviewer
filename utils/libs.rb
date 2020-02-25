#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
libOpenGL.*
libGL\.so
libGLdispatch\.so
libGLX\.so
libX.*
.*nvidia.*
libpthread.*
libresolv.*
libm\.so
libc\.so.*
librt\..*
libdl.*
libxcb.*
libasound.*
libglib.*
libgpg-error.*
libstdc\+\+\.so.*
libgcc_s.*
libfontconfig.*
libfreetype.*
libz.*
)

@options = { :verbose => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/libs.rb [@options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!

EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

def parse( files )

  for line in files
    lib, loc = line.split(" => ")

    next if not loc or not lib

    loc.sub!(/\s*$/, '')
    lib.sub!(/^\s*/, '')

    if loc.empty?
      next
    end

    if lib =~ EXCLUDE_REGEX
      $stderr.puts "Exclude #{lib}" if @options[:verbose]
      next
    end


    begin
      FileUtils.rm("#{@debug}/lib/#{lib}")
    rescue
    end
    if @options[:verbose]
      $stderr.puts "loc: #{loc} -> lib: #{lib}"
    else
      $stderr.print "."
    end
    orig = lib
    if File.symlink?( loc )
      libpath = File.readlink( loc )
      puts "#{loc} ==> #{libpath} ==> #{orig}" if @options[:verbose]
      lib = libpath.gsub(/.*\//, '' )
      puts "#{loc} ==> #{lib} ==> #{orig}" if @options[:verbose]
      FileUtils.cp(loc, "#{@debug}/lib/#{lib}" )
      `chrpath -d "#{@debug}/lib/#{lib}"`
      print `readelf -d #{@debug}/lib/#{lib} | grep PATH`
      FileUtils.ln_s( "#{lib}", "#{@debug}/lib/#{orig}" )
    else
      FileUtils.cp(loc, "#{@debug}/lib/#{lib}" )
      `chrpath -d "#{@debug}/lib/#{lib}"`
      $stderr.print `readelf -d #{@debug}/lib/#{lib} | grep PATH`
    end
  end
end

@debug = ARGV.shift
if not @debug
  @debug = "Release"
elsif not @debug == "Debug"
  $stderr.puts "Invalid option: #$0 [Debug|Release]"
  exit 1
end

def copy_files( build )
  Dir.chdir( '../..'  )
  $stderr.puts "Copy shaders"
  FileUtils.rm_rf( "#{build}/#{@debug}/shaders" )
  FileUtils.cp_r( "shaders/", "#{build}/#{@debug}/" )
  $stderr.puts "Copy docs"
  FileUtils.rm_rf( "#{build}/#{@debug}/docs" )
  FileUtils.cp_r( "docs/", "#{build}/#{@debug}/" )
  FileUtils.rm_rf( "#{build}/#{@debug}/colors" )
  FileUtils.cp_r( "colors/", "#{build}/#{@debug}/" )
  $stderr.puts "Copy ctl scripts"
  FileUtils.rm_rf( "#{build}/#{@debug}/ctl" )
  FileUtils.cp_r( "ctl/", "#{build}/#{@debug}/" )
  $stderr.puts "Copy ocio configs"
  FileUtils.rm_rf( "#{build}/#{@debug}/ocio" )
  FileUtils.cp_r( "ocio/", "#{build}/#{@debug}/" )
  $stderr.puts "Copy otio adapters"
  FileUtils.rm_rf( "#{build}/#{@debug}/otio" )
  FileUtils.cp_r( "otio/", "#{build}/#{@debug}/" )
  if build =~ /Linux/
    # Copy the RED library
    FileUtils.cp_r( "../R3DSDKv7_2_0/Redistributable/linux/REDR3D-x64.so",
                    "#{build}/#{@debug}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Libraries/libBlackmagicRawAPI.so",
                    "#{build}/#{@debug}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++.so.1",
                    "#{build}/#{@debug}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++abi.so.1",
                    "#{build}/#{@debug}/lib" )
  end
end



kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64/"


$stderr.puts "DIRECTORY: #{Dir.pwd}"

if kernel !~ /MINGW.*/
  home=ENV['HOME']+"/bin/mrViewer"
  FileUtils.rm_f( home )
  FileUtils.ln_s( Dir.pwd + '/'+build+"/Release/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer" )
  FileUtils.rm_f( home + '-dbg' )
  FileUtils.ln_s( Dir.pwd + '/'+build+"/Debug/bin/mrViewer.sh", ENV['HOME']+"/bin/mrViewer-dbg" )

  Dir.chdir( build  )
  libs = Dir.glob( "#{@debug}/lib/*" )
  FileUtils.rm_f( libs )

  exes = Dir.glob( "#{@debug}/bin/*" )

  files = []

  for exe in exes
    output=`ldd #{exe}`
    output.gsub!( /\(0x.*\)/, '' )
    files += output.split("\n")
  end

  files.sort!
  files.uniq!

  parse( files )
  copy_files( build )

  $stderr.puts "remove .fuse files"
  `find BUILD/Linux* -name '*fuse*' -exec rm {} \\;`
else
  build = "BUILD/Windows-6.3.9600-64/"
  Dir.chdir( build  )
  copy_files( build )

  build = "BUILD/Windows-6.3.9600-32/"
  Dir.chdir( build  )
  copy_files( build )
end
