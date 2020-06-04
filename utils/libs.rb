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
libdrm2.*
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

@options = { :verbose => false, :libs_only => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/libs.rb [@options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  opts.on("-l", "--libs_only", "Run verbosely") do |v|
    @options[:libs_only] = v
  end

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!

EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

def parse( files, dest )

  for line in files
    lib, loc = line.split(" => ")

    next if not loc or not lib

    loc.strip!
    lib.strip!

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
      FileUtils.cp(loc, "#{dest}/lib/#{lib}" )
      `chrpath -d "#{dest}/lib/#{lib}"`
      print `readelf -d #{dest}/lib/#{lib} | grep PATH`
      FileUtils.ln_s( "#{lib}", "#{dest}/lib/#{orig}" )
    else
      FileUtils.cp(loc, "#{dest}/lib/#{lib}" )
      `chrpath -d "#{dest}/lib/#{lib}"`
      $stderr.print `readelf -d #{dest}/lib/#{lib} | grep PATH`
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

def copy_files( dest )
<<<<<<< HEAD
=======
  Dir.chdir( '../..'  )
>>>>>>> mac
  $stderr.puts "Copy shaders"
  FileUtils.rm_rf( "#{dest}/shaders" )
  FileUtils.cp_r( "shaders/", "#{dest}/" )
  $stderr.puts "Copy docs"
  FileUtils.rm_rf( "#{dest}/docs" )
  FileUtils.rm_rf( "docs/*~" )
  FileUtils.cp_r( "docs/", "#{dest}/" )
<<<<<<< HEAD
  FileUtils.rm_rf( "#{dest}/colors" )
  FileUtils.cp_r( "colors/", "#{dest}/" )
=======
  FileUtils.rm_rf( "#{dest}/share" )
  FileUtils.cp_r( "share/", "#{dest}/", :verbose => true )
  FileUtils.rm_rf( "#{dest}/colors" )
  FileUtils.cp_r( "colors/", "#{dest}/", :verbose => true )
>>>>>>> mac
  $stderr.puts "Copy ctl scripts"
  FileUtils.rm_rf( "#{dest}/ctl" )
  FileUtils.cp_r( "ctl/", "#{dest}/" )
  $stderr.puts "Copy ocio configs"
  FileUtils.rm_rf( "#{dest}/ocio" )
  FileUtils.cp_r( "ocio/", "#{dest}/" )
  $stderr.puts "Copy otio adapters"
  FileUtils.rm_rf( "#{dest}/otio" )
  FileUtils.cp_r( "otio/", "#{dest}/" )
end

def copy_third_party( root, dest )
  Dir.chdir( root )
  if dest =~ /Linux/
    # Copy the RED library
    FileUtils.cp_r( "../R3DSDKv7_2_0/Redistributable/linux/REDR3D-x64.so",
                    "#{dest}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Libraries/libBlackmagicRawAPI.so",
                    "#{dest}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++.so.1",
                    "#{dest}/lib" )
    FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++abi.so.1",
                    "#{dest}/lib" )
  elsif dest =~ /Darwin/
    # Copy the RED library
    FileUtils.cp_r( "../R3DSDKv7_3_1/Redistributable/mac/REDR3D.dylib",
                    "#{dest}/lib/", :verbose => true )
    FileUtils.rm_f( "#{dest}/lib/BlackMagicRAWAPI.framework" )
    FileUtils.ln_s( "/Applications/Blackmagic RAW/Blackmagic RAW SDK/Mac/Libraries/BlackmagicRawAPI.framework/", "#{dest}/lib" )

  end
end


kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64/"

puts "kernel: #{kernel}"

$stderr.puts "DIRECTORY: #{Dir.pwd}"
root = Dir.pwd

if kernel !~ /MINGW.*/

  if build =~ /Linux/
    dest = "#{build}/#@debug"
  elsif build =~ /Darwin/
    dest = "#{build}/#@debug/bin/mrViewer.app/Contents/Resources"
    FileUtils.mkdir_p dest
    FileUtils.mkdir_p ( dest + "/lib/" )
  end

  home=ENV['HOME']+"/bin/mrViewer"
  if build =~ /Linux/
    FileUtils.rm_f( home )
    FileUtils.ln_s( dest + "/Release/bin/mrViewer.sh", home )
    FileUtils.rm_f( home + '-dbg' )
    FileUtils.ln_s( dest + "/Debug/bin/mrViewer.sh", home + "-dbg" )
  end

  Dir.chdir( root  )
  libs = Dir.glob( "#{dest}/lib/*" )
  FileUtils.rm_f( libs )
  exes = Dir.glob( "#{dest}/bin/*" )

  files = []

  if kernel =~ /Linux/
    for exe in exes
      output=`ldd "#{exe}"`
      output.gsub!( /\(0x.*\)/, '' )
      files += output.split("\n")
    end

    files.sort!
    files.uniq!

    parse( files, dest )

  end

  copy_third_party( root, dest )

  if @options[:libs_only]
    exit(0)
  end

  copy_files( dest )

  if kernel =~ /Linux/
    $stderr.puts "remove .fuse files"
    `find BUILD/Linux* -name '*fuse*' -exec rm {} \\;`
  end

elsif kernel =~ /Darwin/
  Dir.chdir( dest  )
  copy_files( dest )
else
  build = "BUILD/Windows-6.3.9600-64/"
  Dir.chdir( build  )
  copy_files( build )

  build = "BUILD/Windows-6.3.9600-32/"
  Dir.chdir( build  )
  copy_files( build )
end
