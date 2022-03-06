#!/usr/bin/env python

import argparse, re, sys

VERSION=0.5

parser = argparse.ArgumentParser(description='3DL LUT to CTL command-line converter.', epilog="Example: 3dl2ctl.py tint.3dl LMT.tint.ctl")
parser.add_argument("--maxValueSpline",
                        help="Maximum Value in the 1D LUT (1023)",
                        type=float, default=1023.0)
parser.add_argument("--maxValue", help="Maximum Value in the 3D LUT (Auto)",
                        type=float, default=0.0)
parser.add_argument("--min", help="Minimum red, green, blue values (0 0 0)",
                        type=float, nargs=3, default=[0.0, 0.0, 0.0])
parser.add_argument("--max", help="Maximum red, green, blue values (1 1 1)",
                        type=float, nargs=3, default=[1.0, 1.0, 1.0])
parser.add_argument('-v', '--version', help="Print version of the script",
                        action="store_true")
parser.add_argument("input", help="Input 3dl file" )
parser.add_argument("output", help="Output CTL file" )
args = parser.parse_args()

version  = args.version
maxValueSpline = args.maxValueSpline
maxValue = args.maxValue
rgbmin   = args.min
rgbmax   = args.max
filename = args.input
output   = args.output

if version:
    print "%s v%.2f" % ( sys.argv[0], VERSION )
    exit(-1)

try:
    with open(filename) as x: lines = x.readlines()
except (IOError, OSError) as e:
    sys.stderr.write( "Could not open '%s' for reading" % filename + ".\n" +
                          str(e) + "\n")
    exit(-1)

m = re.search('\.ctl$', output)
if not m:
    output += '.ctl'

try:
    out = open( output, 'w' )
except (IOError, OSError) as e:
    sys.stderr.write( "Opening output ctl file '%s' failed: " % output + "\n" +
                          str(e) + "\n" )
    exit(-1)

size = [0, 0, 0]
lines2 = []
for i in lines:
    m = re.search( r'(?:Dimensions|Sample)\s+(\d+)x(\d+)x(\d+)', i )
    if m:
        size[0] = int(m.group(1))
        size[1] = int(m.group(2))
        size[2] = int(m.group(3))

    m = re.search( r'Input bit depth (\d+)', i )
    if m:
        idepth = int(m.group(1))

    m = re.search( r'Output bit depth (\d+)', i )
    if m:
        odepth = int(m.group(1))
    if re.search( r'^#.*', i ):
        continue
    if re.search( r'^\s*$', i ):
        continue
    lines2.append(i)

lines = lines2
print filename, '->', output

out.write( '// CTL 3d Lut from %s\n' % filename )
out.write( '// Min: %.6f, %.6f, %.6f\n' % (rgbmin[0], rgbmin[1], rgbmin[2]) )
out.write( '// Max: %.6f, %.6f, %.6f\n' % (rgbmax[0], rgbmax[1], rgbmax[2]) )
out.write( '\n' )

spline = lines.pop(0)

values = spline.split()

if len(values) != size[0]:
    if size[0] != 0:
        sys.stderr.write( 'Number of spline values different than 3D Lut.\n' )
    else:
        size[0] = size[1] = size[2] = len(values)


out.write( '// Lut3D size %dx%dx%d\n' % ( size[0], size[1], size[2] ) )

fvalues = []
for v in values:
    v = int(v) / maxValueSpline
    v = round(v, 12)
    fvalues.append(v)

fvals = ', '.join( map(str, fvalues) )

if len(values) > 0:
    out.write( 'const float spline[%d] = { %s };\n\n' % (len(values), fvals) )

out.write('const float min3d[3] = { %f, %f, %f };\n' %
              (rgbmin[0], rgbmin[1], rgbmin[2] ))
out.write('const float max3d[3] = { %f, %f, %f };\n' %
              (rgbmax[0], rgbmax[1], rgbmax[2] ))

out.write('const float cube[%d][%d][%d][3] = \n' %
              (size[0], size[1], size[2]) )

idx = 0

if len(lines) != size[0]*size[1]*size[2]:
    sys.stderr.write( 'Size of lines %d different than cube size %dx%dx%d' %
                          ( len(lines), size[0], size[1], size[2] ) )
    size[0] = round( len(lines)**(1./3.) )
    size[1] = size[2] = size[0]

maxv = 0
if maxValue == 0.0:
    for line in lines:
        m = re.search( r'^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)', line )
        if int(m.group(1)) > maxv:
            maxv = int(m.group(1))
        if int(m.group(2)) > maxv:
            maxv = int(m.group(2))
        if int(m.group(3)) > maxv:
            maxv = int(m.group(3))

    maxValue = 1023
    if maxv > 1023: maxValue = 2047
    if maxv > 2047: maxValue = 4095

for x in range( 1, size[0]+1 ):
    if x == 1 : out.write( '{ ' )
    for y in range( 1, size[1]+1 ):
        if y == 1: out.write( '{ ' )
        for z in range( 1, size[2]+1 ):
            if z == 1: out.write( '{ ' )
            m = re.search( r'^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)', lines[idx] )
            r = rgbmax[0] * float(m.group(1)) / maxValue
            g = rgbmax[1] * float(m.group(2)) / maxValue
            b = rgbmax[2] * float(m.group(3)) / maxValue
            out.write( '{ %.7f, %.7f, %.7f }' % ( r, g, b ) )
            if z != size[2]:
                out.write( ',\n' )
            idx += 1
        out.write( ' }' )
        if y != size[1]:
            out.write( ',\n' )
        out.write('\n')
    out.write( ' }' )
    if x != size[0]:
        out.write( ',\n' )
    out.write( '\n' )
out.write( '};\n\n' )

out.write( '''
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

''' )

print "MAXIMUM VALUE IN 3D LUT %g, using %g" % ( maxv, maxValue )
