#!/usr/bin/env ruby
# encoding: utf-8

require 'fileutils'
require 'optparse'
require 'shellwords'

EXCLUDE = %w(
SYSTEM32
WINSXS
API-MS
EXT-MS
IESHIMS
)

@options = { :verbose => false, :libs_only => false }
OptionParser.new do |opts|
  opts.banner = "Usage: utils/winlibs.rb [options]"

  opts.on("-w32", "--win32", "win32 version") do |v|
    @options[:win32] = v
  end
  
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
elsif not @debug == "Debug" and not @debug == "Release"
  $stdout.puts "Invalid option: #@debug [Debug|Release]"
  exit 1
end

kernel = `uname`.chop!
release = `uname -r`.chop!

if kernel !~ /MINGW.*/
  $stderr.puts "Not a Windows bash shell"
  exit 1
end

if @options[:win32]
  build = "BUILD/Windows-6.3.9600-32/Release"
else
  build = "BUILD/Windows-6.3.9600-64/Release"
end

@exes = [
  "#{build}/bin/mrViewer.exe",
  "#{build}/bin/oiiotool.exe"
]



@depexe = 'C:/Program Files/depends22_x64/depends.exe'
@deptxt = "#{build}/tmp/dep.txt"

def run( cmd, opts )
  err = system( cmd, *opts )
  if err == nil
    $stderr.puts "#{cmd} not found!"
  elsif err == false
    if "#$?" =~ /exit\s+10$/
      return 0
    end
    $stderr.puts "#{cmd} #{opts} failed! #$?"
  end
  return err
end

def process

  opts = [ "/c", "/of", @deptxt, @exe ]
  if not run( @depexe, opts )
    puts "exit 1"
    exit 1
  end

  lines = File.readlines( @deptxt, :encoding => 'ISO-8859-1' )
  @ignore = []
  @needed = []
  in_ignore = false
  for line in lines
    if line =~ /"KnownDLLs" list/
      in_ignore = true
    end
    if in_ignore
      if line =~ /The application directory/
        in_ignore = false
        next
      end
      line =~ /\[\w+\s*\]\s+([\w\d:\\\/\.]+.dll)/i
      dll = $1
      next if not dll
      dll.upcase!
      @ignore.push( dll )
    else
      line =~ /([\w\d\:\\\-\/\.]+\.dll)/i
      dll = $1
      next if not dll
      dll.upcase!
      if @ignore.any?( dll ) or dll =~ EXCLUDE_REGEX
        next
      end
      @needed.push( dll )
    end
  end

  puts @needed.uniq!.sort!

  for file in @needed
    if file =~ /^FILES/
      puts "MISSING DLL #{file}"
    end
  end
  
end


for @exe in @exes
  process
end
