#!/usr/bin/env ruby

require 'optparse'

VERSION=0.1

rmin = gmin = bmin = 0.0
rmax = gmax = bmax = 1.0


begin


  opt_parser = OptionParser.new do |opts|
    opts.banner = "Usage: cube2ctl.rb [options] <file.cube> <output.ctl>"

    opts.separator ""
 
    opts.on("--min r,g,b", Array, "Mininum red, green, blue values (0,0,0)") do |list|
      rmin, gmin, bmin = list
      rmin = rmin.to_f
      gmin = gmin.to_f
      bmin = bmin.to_f
    end
 
    opts.on("--max r,g,b", Array, "Maximum red, green, blue values (1,1,1)") do |list|
      rmax, gmax, bmax = list
      rmax = rmax.to_f
      gmax = gmax.to_f
      bmax = bmax.to_f
    end
    opts.on_tail("-h", "--help", "Show this message") do
      puts opts
      exit
    end
    
    # Another typical switch to print the version.
    opts.on_tail("--version", "Show version") do
      puts ::VERSION
      exit
    end
  end

  opt_parser.parse!(ARGV)
  
  file = ARGV[0]
  output = ARGV[1]
  
  lines = File.readlines(file)
  out = File.open( output, "w+" )

rescue => e
  puts e
  puts
  puts opt_parser
  exit 1
end

output = output.dup



size = [ 32, 32, 32 ]


for i in lines
  if i =~ /TITLE\s+"([^"]+)"/
    title = $1
  end
  if i =~ /DOMAIN_MIN\s+*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
    rmin = $1.to_f
    gmin = $2.to_f
    bmin = $3.to_f
  end
  if i =~ /DOMAIN_MAX\s+*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
    rmax = $1.to_f
    gmax = $2.to_f
    bmax = $3.to_f
  end
  if i =~ /LUT_3D_SIZE\s+(\d+)/
    size = $1.to_i
  end
end

lines.delete_if { |x| x =~ /^#.*/ }  # remove comments
lines.delete_if { |x| x =~ /^\s*$/ } # remove empty lines
# Remove common command lines
lines.delete_if { |x| x =~ /^\s*LUT_3D_SIZE/ }
lines.delete_if { |x| x =~ /^\s*TITLE/ }
lines.delete_if { |x| x =~ /^\s*DOMAIN/ }


if output !~ /\.ctl$/
  output << '.ctl'
end

puts "#{file} -> #{output}"

out.puts "// #{title}"
out.puts "// CTL 3d Lut from #{file}"
out.puts "// Min: #{rmin}, #{gmin}, #{bmin}"
out.puts "// Max: #{rmax}, #{gmax}, #{bmax}"
out.puts "// Lut3D size #{size}x#{size}x#{size}"
out.puts


out.puts "const float min3d[3] = { #{rmin}, #{gmin}, #{bmin} };"
out.puts "const float max3d[3] = { #{rmax}, #{gmax}, #{bmax} };"


last = size * size * size
if lines.size != last
  $stderr.puts "ERROR: Size of lines #{lines.size} different than cube size #{size}x#{size}x#{size} (#{last})"
end

out.puts "const float cube[#{size}][#{size}][#{size}][3] = "

idx = 0
rgb = []

puts "First line: #{lines[0]}"

for x in 1..size
  for y in 1..size
    for z in 1..size
      lines[idx] =~ /^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
      r = rmax * $1.to_f
      g = gmax * $2.to_f
      b = bmax * $3.to_f
      rgb[x-1] = [] if not rgb[x-1]
      rgb[x-1][y-1] = [] if not rgb[x-1][y-1]
      rgb[x-1][y-1][z-1] = "#{r}, #{g}, #{b}"
      idx += 1
    end
  end
end



for z in 0...size
  out.print "{ " if z == 0
  for y in 0...size
    out.print "{ " if y == 0
    for x in 0...size
      out.print "{ " if x == 0
      out.print "{ #{rgb[x][y][z]} }"
      if x != size-1
        out.print ", "
      end
    end
    out.print " }"
    if y != size-1
      out.print ","
    end
    out.puts
  end
  out.print " }"
  if z != size-1
    out.print ", "
  end
  out.puts
end
out.puts " };"

out.puts
out.puts <<"EOF"

void main( varying float rIn,
           varying float gIn,
           varying float bIn,
           varying float aIn,
           output varying float rOut,
           output varying float gOut,
           output varying float bOut,
           output varying float aOut )
{
lookup3D_f( cube, min3d, max3d, rIn, gIn, bIn, rOut, gOut, bOut );
aOut = aIn;
}

EOF

$stderr.puts "LAST LINE: #{lines[idx]}"
