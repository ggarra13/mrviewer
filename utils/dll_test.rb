#!/usr/bin/env ruby

if ARGV[0]
  dir = ARGV[0]
  arch = 'x64'
else
  dir = "BUILD/Windows*32/Release/bin"
  arch = 'x86'
end

puts "Check #{dir}"

def check_files( files, arch )
  for f in files
    h = `dumpbin -headers #{f}`
    if h =~ /machine.*^#{arch}/
      puts "ERROR: #{f} is wrong architecture when it should be #{arch}"
      if h =~ /debug/i
        puts "ERROR: #{f} is set incorrectly wrong #{arch} debug"
      end
    else
      if h =~ /debug/i
        puts "ERROR: #{f} is set to #{arch} debug"
      end
    end
  end
end


files = Dir.glob(dir + "/*")

check_files( files, arch )

puts "Check BUILD/Windows*64/Release/bin"
files = Dir.glob("BUILD/Windows-6.3.9600-64/Release/bin/*")

check_files( files, 'x64' )

puts "Finished checking BUILD area.  Will check vc14_Windows_* area."

#
#
#

puts "Check lib/vc14_Windows_64/bin"
files = Dir.glob("../../lib/vc14_Windows_64/bin/*")

check_files( files, 'x64' )




puts "Check lib/vc14_Windows_32/bin"
files = Dir.glob("../../lib/vc14_Windows_32/bin/*")

check_files( files, 'x86' )
