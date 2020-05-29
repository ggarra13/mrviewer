#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
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


@debug = ARGV.shift
if not @debug
  @debug = "Release"
elsif not @debug == "Debug"
  $stderr.puts "Invalid option: #$0 [Debug|Release]"
  exit 1
end



kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64/"


$stderr.puts "DIRECTORY: #{Dir.pwd}"

if kernel !~ /Darwin/
  exit 1
end

dest = "#{build}/#@debug/bin/mrViewer.app/Contents"
FileUtils.mkdir_p dest, :mode => 0755
rsrcs = dest + "/Resources"
FileUtils.mkdir_p rsrcs, :mode => 0755
@libdir = rsrcs + "/lib"
FileUtils.mkdir_p @libdir, :mode => 0755

appdir = dest + "/MacOS"
app = appdir + "/mrViewer"

@path = ''
@count = 0

def parse( app )
  @path += ":" + app
  output = `otool -l #{app}`

  lines = output.split("\n")
  for line in lines
    if line =~ /name\s+(.*\.dylib)/
      lib = $1
      if lib =~ /^\/usr\/lib\//
        next
      end
      rpath = lib.sub(/@rpath/, "/usr/local/lib")
      if rpath !~ /\//
        rpath = "/usr/local/lib/" + rpath
      end
      if rpath != lib and @count == 0
        @count += 1
        puts rpath
        parse rpath
        next
      end
      if @count == 1
        @count = 0
        next
      end
      puts "cp_r #{lib} #@libdir"
      FileUtils.cp_r lib, @libdir
      lib =~ /\/([\w\d\-_\.]+\.dylib)/
      libname = $1
      newlib = "#@libdir/#{libname}"
      FileUtils.chmod 0755, newlib
      `install_name_tool -change "#{lib}" "@rpath/#{libname}" "#{newlib}"`
    end
  end
end


parse app
