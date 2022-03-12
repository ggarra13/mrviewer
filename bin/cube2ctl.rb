#!/usr/bin/env ruby

require 'optparse'

version, patch, teeny = RUBY_VERSION.split('.').map { |e| e.to_i }
if version <= 1 and patch <= 8
  $stderr.puts "The #{$0} script requires ruby1.9 or later."
  $stderr.puts "Your ruby version is #{RUBY_VERSION}."
  exit 1
end

Version=0.3

maxValueSpline = 1.0
rmin = gmin = bmin = 0.0
rmax = gmax = bmax = 1.0


begin


  opt_parser = OptionParser.new do |opts|
    opts.banner = "CUBE LUT to CTL command-line converter.\n\n" +
    "Usage: cube2ctl.rb [options] <file.cube> <output.ctl>"

    opts.separator ""
 
    opts.on("--maxValueSpline [N]", Float, "Maximum value in spline (1.0)") do |val|
      maxValueSpline = val
    end
    
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
    opts.on_tail("-v", "--version", "Show version") do
      puts "#$0 v#{::Version}"
      exit
    end
  end

  opt_parser.parse!(ARGV)
  
  file = ARGV[0]
  output = ARGV[1]
  
  lines = File.readlines(file)
  output = output.dup
  
  if output !~ /\.ctl$/
    output << '.ctl'
  end
  
  out = File.open( output, "w" )

rescue TypeError => e
  if not file
    $stderr.puts "No input file specified!"
  elsif not output
    $stderr.puts "No output file specified!"
  else
    $stderr.puts e
  end
  puts
  puts opt_parser
  exit -1
rescue => e
  $stderr.puts "Could not open file. " + e.to_s
  puts
  puts opt_parser
  exit -1
end



lut1d = false
size = [ 32, 32, 32 ]


for i in lines
  if i =~ /TITLE\s+"([^"]+)"/
    title = $1
  end
  if i =~ /DOMAIN_MIN\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
    rmin = $1.to_f
    gmin = $2.to_f
    bmin = $3.to_f
  end
  if i =~ /DOMAIN_MAX\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
    rmax = $1.to_f
    gmax = $2.to_f
    bmax = $3.to_f
  end
  if i =~ /LUT_1D_SIZE\s+(\d+)/
    size = $1.to_i
    lut1d = true
  end
  if i =~ /LUT_3D_SIZE\s+(\d+)/
    size = $1.to_i
  end
end

lines.delete_if { |x| x =~ /^#.*/ }  # remove comments
lines.delete_if { |x| x =~ /^\s*$/ } # remove empty lines
# Remove common command lines
lines.delete_if { |x| x =~ /^\s*LUT_1D_SIZE/ }
lines.delete_if { |x| x =~ /^\s*LUT_3D_SIZE/ }
lines.delete_if { |x| x =~ /^\s*TITLE/ }
lines.delete_if { |x| x =~ /^\s*DOMAIN/ }
lines.delete_if { |x| x =~ /CUBE/ }



puts "#{file} -> #{output}"

out.puts "// #{title}"
out.puts "// CTL 3d Lut from #{file}"
out.puts "// Min: %.6f, %.6f, %.6f" % [rmin, gmin, bmin ]
out.puts "// Max: %.6f, %.6f, %.6f" % [rmax, gmax, bmax ]
if lut1d
  out.puts "// Lut1D size #{size}"
  if lines.size != size
    $stderr.puts "ERROR: Size of lines #{lines.size} different than 1d lut size #{size}"
    if size == 0
      size = lines.size
    end
  end
  out.puts
else
  out.puts "// Lut3D size #{size}x#{size}x#{size}"
  out.puts "const float min3d[3] = { %.6f, %.6f, %.6f };" % [rmin, gmin, bmin ]
  out.puts "const float max3d[3] = { %.6f, %.6f, %.6f };" % [rmax, gmax, bmax ]
  out.puts
  
  last = size * size * size
  if lines.size != last
    $stderr.puts "ERROR: Size of lines #{lines.size} different than cube size #{size}x#{size}x#{size} (#{last})"
  end

  out.puts "const float cube[#{size}][#{size}][#{size}][3] = "
end



if lut1d
  r = []
  g = []
  b = []
  for x in 0...size
    lines[x] =~ /^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
    r << $1.to_f / maxValueSpline
    g << $1.to_f / maxValueSpline
    b << $1.to_f / maxValueSpline
  end

  out.print "const float splineR[#{size}] = { "
  out.puts r.join(', ')
  out.puts "};"
  out.puts
  out.print "const float splineG[#{size}] = { "
  out.puts g.join(', ')
  out.puts "};"
  out.puts
  out.print "const float splineB[#{size}] = { "
  out.puts b.join(', ')
  out.puts "};"
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
  rOut = lookup1D( splineR, #{rmin}, #{rmax}, rIn );
  gOut = lookup1D( splineG, #{gmin}, #{gmax}, gIn );
  bOut = lookup1D( splineB, #{bmin}, #{bmax}, bIn );
  aOut = aIn;
}

EOF
  
else


  idx = 0
  rgb = []

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
          out.print ",\n"
        end
      end
      out.print " }"
      if y != size-1
        out.print ",\n"
      end
    end
    out.print " }"
    if z != size-1
      out.print ",\n"
    end
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
end
