#!/usr/bin/env ruby


puts "Check BUILD/Windows*32/Release/bin"

files = Dir.glob("BUILD/Windows-6.3.9600-32/Release/bin/*")

for f in files
  h = `dumpbin -headers #{f}`
  if h =~ /machine.*x64/
    puts "ERROR: #{f} is x64 when it should be x86"
  end
end

puts "Check BUILD/Windows*64/Release/bin"
files = Dir.glob("BUILD/Windows-6.3.9600-64/Release/bin/*")

for f in files
  h = `dumpbin -headers #{f}`
  if h =~ /machine.*x86/
    puts "ERROR: #{f} is x86 when it should be x64"
  end
end

puts "Finished checking BUILD area.  Will check vc14_Windows_* area."

#
#
#

puts "Check lib/vc14_Windows_64/bin"
files = Dir.glob("../../lib/vc14_Windows_64/bin/*")

for f in files
  h = `dumpbin -headers #{f}`
  if h =~ /machine.*x86/
    puts "ERROR: #{f} is x86 when it should be x64"
  end
end



puts "Check lib/vc14_Windows_32/bin"
files = Dir.glob("../../lib/vc14_Windows_32/bin/*")

for f in files
  h = `dumpbin -headers #{f}`
  if h =~ /machine.*x64/
    puts "ERROR: #{f} is x64 when it should be x86"
  end
end
