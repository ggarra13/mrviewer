#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'


def run_cmd(args)
  puts args
  `#{args}`
end


@options = { :verbose => false, :dir => '.' }
opts = OptionParser.new do |o|
  o.banner = "Usage: utils/dll2lib.rb [options] <file.dll>"

  o.on("-v", "--[no-]verbose", "Run verbosely") do |v|
    @options[:verbose] = v
  end

  o.on("-o", "--output dir" ) do |dir|
    @options[:dir] = dir
  end

  o.on_tail("-h", "--help", "Show this message") do
    $stderr.puts o
    exit
  end
end

opts.parse!


if ARGV.empty?
  $stderr.puts opts
  exit 1
end


dllfile = ARGV.shift

if not File.exists?( dllfile )
  $stderr.puts "#{dllfile} does not exist!"
  exit 1
end

if dllfile !~ /\.dll$/i
  $stderr.puts "#{dllfile} is not a .dll file!"
  exit 1
end

library = dllfile.sub( /\.dll$/i, '' )
if @options[:dir]
  library.gsub!( /.*\//, @options[:dir] + '/' )
end
tmpfile = "#{library}.tmp"
deffile = "#{library}.def"
libfile = "#{library}.lib"

#
# Find out whether the dll is for x86 or x64 machines
#
machine = run_cmd( "dumpbin -headers \"#{dllfile}\"")
if machine =~ /x86/m
  @options[:machine] = 'x86'
elsif machine =~ /x64/m
  @options[:machine] = 'x64'
else
  $stderr.puts "DLL for an architecture not supported: " + @options[:machine].to_s
  exit 1
end

puts
puts "MACHINE ARCHITECTURE OF DLL: #{@options[:machine]}"
puts
  
run_cmd( "dumpbin /EXPORTS \"#{dllfile}\" > \"#{tmpfile}\"" )

#
# Read all lines of tmpfile
#
fin = File.open( "#{tmpfile}", "r" )
lines = fin.readlines
fin.close

#
# Now, parse lines and create .def file
#
fout = File.open( "#{deffile}", "w" )

# Strip paths from library, leaving just its name
library.sub!(/.*\//, '')

fout.puts "LIBRARY #{library}"
fout.puts "EXPORTS"

in_functions = false
for line in lines
  if line =~ /ordinal\s+hint\s+RVA\s+name/
    in_functions = true
  end
  next if not in_functions
  
  break if line =~ /Summary/  # we finished!
  next if line =~ /^\s*$/

  if line =~ /\s*\d+\s+\w+\s+\w+\s+(\w+)/
    function = $1
    if @options[:verbose]
      puts function
    end
    fout.puts function
  end
end

fout.close

#
# Pass def file to lib.exe to create the lib file
#
run_cmd( "lib -def:#{deffile} -out:#{libfile} -machine:#{@options[:machine]}" )

FileUtils.rm_f deffile
FileUtils.rm_f tmpfile

