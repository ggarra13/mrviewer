#!/usr/bin/env ruby

require 'optparse'

version, patch, teeny = RUBY_VERSION.split('.').map { |e| e.to_i }
if version <= 1 and patch <= 8
  $stderr.puts "The #{$0} script requires ruby1.9 or later."
  $stderr.puts "Your ruby version is #{RUBY_VERSION}."
  exit 1
end

Version=0.5

idepth = 10
odepth = 10
maxValue = 0.0
maxValueSpline = 1023.0
rmin = gmin = bmin = 0.0
rmax = gmax = bmax = 1.0


begin


  opt_parser = OptionParser.new do |opts|
    opts.banner =  "3DL LUT to CTL command-line converter.\n\n" +
                   "Usage: 3dl2ctl.rb [options] <file.3dl> <output.ctl>"

    opts.separator ""
    opts.separator "Specific options:"
    
    opts.on("--maxValueSpline [N]", Float, "Maximum Value in the 1D LUT (1023)") do |n|
      maxValueSpline = n
    end
    
    opts.on("--maxValue [N]", Float, "Maximum Value in the 3D LUT (Auto)") do |n|
      maxValue = n
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
  if lines.empty?
    $stderr.puts "Empty input file specified!"
    exit 1
  end

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
rescue StandardError => e
  $stderr.puts "Could not open file. " + e.to_s
  puts
  puts opt_parser
  exit -1
end




size = [ 0, 0, 0 ]


for i in lines
  if i =~ /(?:Dimensions|Sample)\s+(\d+)x(\d+)x(\d+)/
    size[0] = $1.to_i
    size[1] = $2.to_i
    size[2] = $3.to_i
  end
  if i =~ /Input bit depth (\d+)/
    idepth = $1.to_i
  end
  if i =~ /Output bit depth (\d+)/
    odepth = $1.to_i
  end
end

lines.delete_if { |x| x =~ /^#.*/ }  #remove comment lines
lines.delete_if { |x| x =~ /^\s*$/ } #remove empty lines



puts "#{file} -> #{output}"

out.puts "// CTL 3d Lut from #{file}"
out.puts "// Min: %.6f, %.6f, %.6f" % [rmin, gmin, bmin ]
out.puts "// Max: %.6f, %.6f, %.6f" % [rmax, gmax, bmax ]
out.puts
spline = lines[0]


values = spline.split(/\s+/)

if values.size != size[0]
  if size[0] != 0
    $stderr.puts "Number of spline values differrent than 3D Lut."
  else
    size[0] = size[1] = size[2] = values.size
  end
end
out.puts "// Lut3D size #{size[0]}x#{size[1]}x#{size[2]}"

fvalues = []
for v in values
  v = v.to_i / maxValueSpline
  fvalues << v.round(12)
end

fvals = fvalues.join(', ')

if not values.empty?
  out.puts "const float spline[#{values.size}] = { #{fvals} };"
  out.puts
end

lines.shift

out.puts "const float min3d[3] = { %.6f, %.6f, %.6f };" % [rmin, gmin, bmin ]
out.puts "const float max3d[3] = { %.6f, %.6f, %.6f };" % [rmax, gmax, bmax ]

out.puts "const float cube[#{size[0]}][#{size[1]}][#{size[2]}][3] = "

idx = 0

if lines.size != size[0] * size[1] * size[2]
  $stderr.puts "Size of lines #{lines.size} different than cube size #{size[0]}x#{size[1]}x#{size[2]}"
  size[0] = Math.cbrt( lines.size )
  size[0] = size[1] = size[2] = size[0].to_i
end

max = 0
if maxValue == 0.0
  for line in lines
      line =~ /^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
      max = $1.to_i if $1.to_i > max
      max = $2.to_i if $2.to_i > max
      max = $3.to_i if $3.to_i > max
  end

  maxValue = 1023
  maxValue = 2047 if max > 1023
  maxValue = 4095 if max > 2047
end

for x in 1..size[0]
  out.print "{ " if x == 1
  for y in 1..size[1]
    out.print "{ " if y == 1
    for z in 1..size[2]
      out.print "{ " if z == 1
      lines[idx] =~ /^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)/
      r = rmax * $1.to_f / maxValue
      g = gmax * $2.to_f / maxValue
      b = bmax * $3.to_f / maxValue
      out.print "{ %.7f, %.7f, %.7f }" % [ r, g, b ]
      if z != size[2]
        out.print ",\n"
      end
      idx += 1
    end
    out.print " }"
    if y != size[1]
      out.print ",\n"
    end
    out.puts
  end
  out.print " }"
  if x != size[0]
    out.print ",\n"
  end
  out.puts
end
out.puts " };"

out.puts

out.puts <<'EOF'

void main( varying float rIn,
           varying float gIn,
           varying float bIn,
           varying float aIn,
           output varying float rOut,
           output varying float gOut,
           output varying float bOut,
           output varying float aOut )
{
  float rgb[3];
  rgb[0] = lookup1D( spline, 0.0, 1.0, rIn );
  rgb[1] = lookup1D( spline, 0.0, 1.0, gIn );
  rgb[2] = lookup1D( spline, 0.0, 1.0, bIn );
  lookup3D_f( cube, min3d, max3d, rgb[0], rgb[1], rgb[2], rOut, gOut, bOut );
  aOut = aIn;
}

EOF


puts "MAXIMUM VALUE IN 3D LUT #{max}, using #{maxValue}"
