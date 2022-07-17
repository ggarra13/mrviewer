#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
libOpenGL.*
libGL\.so
libGLdispatch\.so
libGLX\.s
libX.*
.*nvidia.*
libdrm2.*
libpthread.*
libresolv.*
libm\.so
librt\..*
libdl.*
libxcb.*
libasound.*
libgpg-error.*
libfontconfig.*
libfreetype.*
libharfbuzz.*
libxshmfence.*
libz.*
libc\.so.*
#libglib.*
libstdc\+\+\.so.*
libgcc_s.*
)

@options = { :verbose => false, :libs_only => false, :force => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/libs.rb [@options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  opts.on("-f", "--force", "Force lib copy") do |v|
    @options[:force] = v
  end

  opts.on("-t", "--translations_only", "Copy translations only") do |v|
    @options[:translations_only] = v
  end

  opts.on("-l", "--libs_only", "Copy libs only") do |v|
    @options[:libs_only] = v
  end

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!

EXCLUDE_REGEX = /(?:#{EXCLUDE.join('|')}).*/

def parse( files, dest )
  return if @options[:translations_only]

  for line in files
    lib, loc = line.split(" => ")

    $stdout.puts "#{lib} -> #{loc}" if @options[:verbose]
    if not loc or not lib or loc =~ /not found/
      $stderr.puts "LIBRARY #{lib} NOT FOUND!"
      next
    end

    loc.strip!
    lib.strip!

    if loc.empty?
      next
    end

    if lib =~ EXCLUDE_REGEX
      $stdout.puts "Exclude #{lib}" if @options[:verbose]
      next
    end


    begin
      FileUtils.rm("#{@debug}/lib/#{lib}")
    rescue
    end
    if @options[:verbose]
      $stdout.puts "loc: #{loc} -> lib: #{lib}"
    else
      $stdout.print "."
    end
    orig = lib
    if File.symlink?( loc )
      libpath = File.readlink( loc )
      puts "#{loc} ==> #{libpath} ==> #{orig}" if @options[:verbose]
      lib = libpath.gsub(/.*\//, '' )
      puts "#{loc} ==> #{lib} ==> #{orig}" if @options[:verbose]
      FileUtils.cp(loc, "#{dest}/lib/#{lib}", :verbose => @options[:verbose] )
      `chrpath -d "#{dest}/lib/#{lib}"`
      print `readelf -d #{dest}/lib/#{lib} | grep PATH`
      if not File.exists?( "#{dest}/lib/#{orig}" )
        FileUtils.ln_s( "#{lib}", "#{dest}/lib/#{orig}", :verbose => @options[:verbose] )
      end
    else
      FileUtils.cp(loc, "#{dest}/lib/#{lib}", :verbose => @options[:verbose] )
      `chrpath -d "#{dest}/lib/#{lib}"`
      $stdout.print `readelf -d #{dest}/lib/#{lib} | grep PATH`
    end
  end
end

@debug = ARGV.shift
if not @debug
  @debug = "Release"
elsif not @debug == "Debug" and not @debug == "Release" and
    not @debug == "RelWithDebInfo"
  $stdout.puts "Invalid option: #@debug [Debug|RelWithDebInfo|Release]"
  exit 1
end

def copy_files( dest )
  $stdout.puts "Copy shaders"
  FileUtils.rm_rf( "#{dest}/shaders" )
  FileUtils.cp_r( "shaders/", "#{dest}/", :verbose => true )
  $stdout.puts "Copy docs"
  FileUtils.rm_rf( "#{dest}/docs" )
  FileUtils.rm_rf( "docs/*~" )
  FileUtils.cp_r( "docs/", "#{dest}/", :verbose => true )
  FileUtils.rm_rf( "#{dest}/share" )
  FileUtils.cp_r( "share/", "#{dest}/", :verbose => true )
  FileUtils.rm_rf( "#{dest}/colors" )
  FileUtils.cp_r( "colors/", "#{dest}/", :verbose => true )
  # $stdout.puts "Copy ctl scripts"
  FileUtils.rm_rf( "#{dest}/ctl" )
  FileUtils.cp_r( "ctl/", "#{dest}/", :verbose => true )
  $stdout.puts "Copy ocio configs"
  FileUtils.rm_rf( "#{dest}/ocio" )
  FileUtils.cp_r( "ocio/", "#{dest}/", :verbose => true )
  $stdout.puts "Copy otio adapters"
  FileUtils.rm_rf( "#{dest}/otio" )
  FileUtils.cp_r( "otio/", "#{dest}/", :verbose => true )
end

def copy_third_party( root, dest )
  Dir.chdir( root )
  if dest =~ /Linux/
    # Copy the RED library
    FileUtils.cp_r( "../R3DSDKv8_1_0/Redistributable/linux/REDR3D-x64.so",
                    "#{dest}/lib" )
    if File.exists?( "/usr/lib64/blackmagic" )
      FileUtils.cp_r( "/usr/lib64/blackmagic/BlackmagicRAWSDK/Linux/Libraries/libBlackmagicRawAPI.so",
                      "#{dest}/lib" )
      FileUtils.cp_r( "/usr/lib64/blackmagic/BlackmagicRAWSDK/Linux/Libraries/libc++.so.1",
                      "#{dest}/lib" )
      FileUtils.cp_r( "/usr/lib64/blackmagic/BlackmagicRAWSDK/Linux/Libraries/libc++abi.so.1",
                      "#{dest}/lib" )
    else
      FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Libraries/libBlackmagicRawAPI.so",
                      "#{dest}/lib" )
      FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++.so.1",
                      "#{dest}/lib" )
      FileUtils.cp_r( "../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/Linux/Samples/ExtractFrame/libc++abi.so.1",
                      "#{dest}/lib" )
    end
  elsif dest =~ /Darwin/
    force = ''
    if @options[:force]
      force = '-f'
    end
    if not system( "#{root}/utils/maclibs.rb #{force} #@debug" )
      exit 1
    end
    # Copy the RED library
    FileUtils.cp_r( "../R3DSDKv8_1_0/Redistributable/mac/REDR3D.dylib",
                    "#{dest}/lib/", :verbose => true )
    FileUtils.rm_f( "#{dest}/lib/BlackMagicRAWAPI.framework" )
    FileUtils.cp_r( "/Applications/Blackmagic RAW/Blackmagic RAW SDK/Mac/Libraries/BlackmagicRawAPI.framework/", "#{dest}/lib", :verbose => true )
  elsif dest =~ /Windows.*-64/
    FileUtils.cp_r( "#{root}/../../lib/vc14_Windows_64/lib/REDR3D-x64.dll",
                    "#{dest}/lib", :verbose => true )
    FileUtils.cp_r( "#{root}/../../lib/vc14_Windows_64/lib/BlackMagicRawAPI.dll",
                    "#{dest}/lib", :verbose => true )
  elsif dest =~ /Windows.*-32/
    FileUtils.cp_r( "#{root}/../../lib/vc14_Windows_32/lib/REDR3D-x86.dll",
                    "#{dest}/lib", :verbose => true )
  end
end


kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64/"

$stdout.puts "kernel: #{kernel}"

root = $0.sub(/utils\/libs.rb/, "")
if root.size <= 1
  root = Dir.pwd
end
$stdout.puts "DIRECTORY: #{root}"





if kernel !~ /MINGW.*/

  if build =~ /Linux/
    dest = "#{build}/#@debug"
  elsif build =~ /Darwin/
    dest = "#{build}/#@debug"
    FileUtils.mkdir_p dest
    FileUtils.mkdir_p dest + "/lib/"
  end

  home=ENV['HOME']+"/bin/mrViewer"
  if build =~ /Linux/
    FileUtils.rm_f( home )
    FileUtils.ln_s( Dir.pwd + "/#{build}/Release/bin/mrViewer.sh", home,
                    :verbose => true )
    FileUtils.rm_f( home + '-dbg' )
    if File.exists?( Dir.pwd + '/' + "/#{build}/Debug/bin/mrViewer.sh" )
      FileUtils.ln_s( Dir.pwd + '/' + "/#{build}/Debug/bin/mrViewer.sh", home + "-dbg",
                      :verbose => true )
    end
  end

  Dir.chdir( root  )
  libs = Dir.glob( "#{dest}/lib/*" )
  libs.map! { |x| x if x !~ /.*libACESclip.*/ }.compact!
  FileUtils.rm_f( libs ) if not @options[:translations_only]
  exes = Dir.glob( "#{dest}/bin/*" )

  files = []

  if kernel =~ /Linux/ and not @options[:translations_only]
    for exe in exes
      puts "PARSING #{exe}"
      output=`ldd "#{exe}"`
      output.gsub!( /\(0x.*\)/, '' )
      files += output.split("\n")
    end

    files.sort!
    files.uniq!

    parse( files, dest )

  end

  copy_third_party( root, dest ) if not @options[:translations_only]

  if @options[:libs_only]
    exit(0)
  end

  Dir.chdir( root  )
  copy_files( dest )

  if @options[:translations_only]
    exit(0)
  end

  Dir.chdir( dest + "/lib" )
  if kernel =~ /Linux/
    FileUtils.ln_s "libACESclip.so.0.2.6",
                  "libACESclip.so", :verbose => true, :force => true
    FileUtils.ln_s "libAMF.so.0.1.0", "libAMF.so", :verbose => true,
                   :force => true
    Dir.chdir( root )
  elsif kernel =~ /Darwin/
    FileUtils.ln_s "libACESclip.dylib.0.2.6",
                   "libACESclip.dylib", :verbose => true, :force => true
    FileUtils.ln_s "libAMF.dylib.0.1.0",
                   "libAMF.dylib", :verbose => true, :force => true
  end
  Dir.chdir( root )

else

  cl = `cl.exe 2>&1`
  cl = cl.force_encoding( "ISO-8859-1" )
  if cl =~ /x64/
    build = "BUILD/Windows-6.3.9600-64/"
    dest  = "#{build}/#@debug"
    Dir.chdir( root  )
    copy_files( dest )
    copy_third_party( root, dest ) if not @options[:translations_only]
  else
    build = "BUILD/Windows-6.3.9600-32/"
    dest  = "#{build}/#@debug"
    Dir.chdir( root  )
    copy_files( dest )
    copy_third_party( root, dest ) if not @options[:translations_only]
  end
end

exit(0)
