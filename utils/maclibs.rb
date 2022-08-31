#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'

EXCLUDE = %w(
libclang_rt.asan_osx_dynamic.*
)

@options = { :verbose => false, :libs_only => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/maclibs.rb [@options] [Debug|Release]"

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
if @debug
  @debug = @debug[0].upcase + @debug[1,10]
end

if not @debug
  @debug = "Release"
elsif not @debug == "Debug" and not @debug == "Release"
  $stderr.puts "Invalid option: #@debug [Debug|Release]"
  exit 1
end




kernel = `uname`.chop!
if kernel !~ /Darwin/
  exit 1
end
release = `uname -r`.chop!
build = "BUILD/#{kernel}-#{release}-64/"
root = $0.sub(/utils\/maclibs.rb/, "")
if root.size <= 1
  root = Dir.pwd
end

$stdout.puts "kernel: #{kernel}"

$stdout.puts "DIRECTORY: #{root}"

if not @options[:prefix]
  @options[:prefix]="#{root}/install-#{kernel}-#{release}-64/#{@debug}"
end



@dest = "#{build}/#@debug/"
@libdir = "#{@dest}/lib"
FileUtils.mkdir_p @dest, :mode => 0755
libs = Dir.glob( "#{@libdir}/*" )
libs.map! { |x| x if x !~ /.*(?:AMF|ACESclip).*/ }.compact!
FileUtils.rm_rf( libs )

appdir = @dest + "/bin"
app = appdir + "/mrViewer"

objc_opt_self = `nm "#{app}"`.chop!

# if objc_opt_self =~ /objc_opt_self/ and not @options[:force]
#   $stderr.puts "MAC APPLICATION NOT COMPATIBLE WITH MOJAVE"
#   exit 1
# end

@searched_libs = []

def find_lib( lib )
  if @searched_libs.one? lib
    return ''
  end
  @searched_libs.push lib
  libpath = ''
  if @options[:prefix]
    libpath = `find #{@options[:prefix]} -name "#{lib}"`
  end
  if libpath.strip.empty?
    libpath = `find /System/Volumes/Data/usr/local/Cellar/ -name "#{lib}"`
  end
  if libpath.strip.empty? and lib !~ /.*(?:AMF|ACESclip).*/
    $stderr.puts "ERROR: #{lib} WAS NOT FOUND!"
    exit 1
  end
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


  begin
    $stderr.puts app
    output = `otool -L #{app}`
  rescue => e
    $stderr.puts e
  end

  lines = output.split("\n")
  lines = lines[1..lines.size]
  if not lines
    $stderr.puts "#{app} has no libs"
    return
  end

  for line in lines
    lib = line.sub(/^\s+/, '')
    lib = lib.sub(/\(.*\)$/, '')
    lib.strip!
    if lib =~ EXCLUDE_REGEX
      puts "SKIPPING #{lib}"
      next
    end
    if lib =~ /^\/usr\/lib\// or lib =~ /\/System/
      next
    end
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
