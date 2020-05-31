#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
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


@debug = ARGV.shift
if not @debug
  @debug = "Release"
elsif not @debug == "Debug"
  $stderr.puts "Invalid option: #$0 [Debug|Release]"
  exit 1
end



kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64"


$stderr.puts "DIRECTORY: #{Dir.pwd}"

if kernel !~ /Darwin/
  exit 1
end

dest = "#{build}/#@debug/bin/mrViewer.app/Contents"
FileUtils.mkdir_p dest, :mode => 0755
rsrcs = dest + "/Resources"
FileUtils.mkdir_p rsrcs, :mode => 0755
@libdir = rsrcs + "/lib"
FileUtils.rm_rf @libdir
FileUtils.mkdir_p @libdir, :mode => 0755

appdir = dest + "/MacOS"
app = appdir + "/mrViewer"


def find_lib( lib )
  lib = `find /System/Volumes/Data/usr/local/Cellar/ -name "#{lib}"`
  puts lib
  return lib
end

def copy( file, dest )
  begin
    puts "cp #{file}, #{dest}"
    file =~ /\/([\w\d\-_\.]+\.dylib)/
    libname = $1
    newlib = "#{dest}/#{libname}"
    FileUtils.rm_f newlib
    FileUtils.cp_r file, dest
    FileUtils.chmod 0755, newlib
    `install_name_tool -change "#{file}" "\@rpath/#{libname}" "#{newlib}"`
  rescue => e
    $stderr.puts "Could not copy #{file}: #{e}"
  end
end

@files = []

def parse( app )
  if app =~ EXCLUDE_REGEX
    return
  end

  begin
    output = `otool -L #{app}`
  rescue => e
    $stderr.puts e
  end

  lines = output.split("\n")
  lines = lines[1..lines.size]

  for line in lines
    lib = line.sub(/^\s+/, '')
    lib = lib.sub(/\(.*\)$/, '')
    lib.strip!
    if lib =~ /^\/usr\/lib\// or lib =~ /\/System/
      next
    end
    if lib =~ /@loader_path/
      lib.sub!(/@loader_path\//, "")
      lib = find_lib lib
    end
    rpath = lib.sub(/@(?:rpath|loader_path)/, "/usr/local/lib")
    if rpath !~ /\//
      rpath = "/usr/local/lib/" + rpath
    end
    if not @files.one? rpath
      @files.push rpath
      parse rpath
    end
  end
end



parse app

for file in @files
  copy( file, @libdir )
end
