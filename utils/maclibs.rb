#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
libz.*
libclang_rt.asan_osx_dynamic.*
)

@options = { :verbose => false, :libs_only => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/maclibs.rb [@options]"

  opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  opts.on("-f", "--force", "Force library copy") do |v|
    @options[:force] = v
  end

  opts.on("-p PATH", "--prefix=PATH", "installed prefix") do |v|
    @options[:prefix] = v
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
elsif not @debug == "Debug" and not @debug == "Release" and not @debug == "RelWithDebInfo"
  $stderr.puts "Invalid option: #@debug [Debug|Release|RelWithDebInfo]"
  exit 1
end



kernel = `uname`.chop!
release = `uname -r`.chop!

build = "BUILD/#{kernel}-#{release}-64"

puts "PREFIX = #{@options[:prefix]}"

puts "DIRECTORY: #{Dir.pwd}"

if kernel !~ /Darwin/
  exit 1
end

dest = "#{build}/#@debug/"
FileUtils.mkdir_p dest, :mode => 0755
@libdir = dest + "/lib"
FileUtils.rm_rf @libdir
FileUtils.mkdir_p @libdir, :mode => 0755

appdir = dest + "/bin"
app = appdir + "/mrViewer"

objc_opt_self = `nm "#{app}"`.chop!

if objc_opt_self =~ /objc_opt_self/ and not @options[:force]
  $stderr.puts "MAC APPLICATION NOT COMPATIBLE WITH MOJAVE"
  exit 1
end

@searched_libs = []

def find_lib( lib )
  if @searched_libs.one? lib
    return ''
  end
  @searched_libs.push lib
  libpath = `find /System/Volumes/Data/usr/local/Cellar/ -name "#{lib}"`
  if libpath.strip.empty? and @options[:prefix]
    libpath = `find #{@options[:prefix]} -name "#{lib}"`
  end
  if libpath.strip.empty?
    libpath = "/usr/local/lib/#{lib}"
  end
  $stderr.puts libpath
  return libpath.strip
end

def copy( file, dest )
  begin
    file =~ /\/([\w\d\-_\.]+\.dylib)/
    libname = $1
    newlib = "#{dest}/#{libname}"
    FileUtils.rm_rf newlib
    FileUtils.cp_r  file, dest, :verbose => true
    FileUtils.chmod 0755, newlib
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
    $stderr.puts lib
    if lib =~ /@(?:rpath|loader_path)\//
      lib.sub!(/@(?:rpath|loader_path)\//, "")
    end
    if lib !~ /^\//
      lib = find_lib lib
      if lib.empty?
        next
      end
    end
    if not @files.one? lib
      @files.push lib
      parse lib
    end
  end
end



parse app

@files.uniq!
for file in @files
  copy( file, @libdir )
end
