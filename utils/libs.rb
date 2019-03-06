#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

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
libglib.*
libfontconfig.*
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
      lib = File.readlink( loc )
      puts "#{lib} ->->-> #{orig}" if @options[:verbose]
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
end

release = `uname -r`.chop!

build = "BUILD/Linux-#{release}-64/"


$stderr.puts "DIRECTORY: #{Dir.pwd}"

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

Dir.chdir( '../..'  )
$stderr.puts "Copy shaders"
FileUtils.rm_f( "#{@debug}/shaders" )
FileUtils.cp_r( "shaders/", "#{build}/#{@debug}/" )
$stderr.puts "Copy docs"
FileUtils.rm_f( "#{@debug}/docs" )
FileUtils.cp_r( "docs/", "#{build}/#{@debug}/" )
$stderr.puts "Copy ctl scripts"
FileUtils.rm_f( "#{@debug}/ctl" )
FileUtils.cp_r( "ctl/", "#{build}/#{@debug}/" )

$stderr.puts "remove .fuse files"
`find BUILD/Linux* -name '*fuse*' -exec rm {} \\;`
