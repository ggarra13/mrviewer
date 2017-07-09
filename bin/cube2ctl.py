#!/usr/bin/env python

import argparse, re, sys

parser = argparse.ArgumentParser(description="CUBE LUT to CTL command-line converter.", epilog='Example: cube2ctl.py tint.cube LMT.tint.ctl')
parser.add_argument("--maxValueSpline",
                        help="Maximum Value in the 1D LUT (1023)",
                        type=float, default=1.0)
parser.add_argument("--min", help="Minimum red, green, blue values (0 0 0)",
                        type=float, nargs=3, default=[0.0, 0.0, 0.0])
parser.add_argument("--max", help="Maximum red, green, blue values (1 1 1)",
                        type=float, nargs=3, default=[1.0, 1.0, 1.0])
parser.add_argument("input", help="Input .cube file" )
parser.add_argument("output", help="Output CTL file" )
args = parser.parse_args()

maxValueSpline = args.maxValueSpline
rgbmin   = args.min
rgbmax   = args.max
filename = args.input
output   = args.output

with open(filename) as x: lines = x.readlines()

m = re.search('\.ctl$', output)
if not m:
    output += '.ctl'

try:
    out = open( output, 'w' )
except (IOError, OSError) as e:
    sys.stderr.write( "Could not open ctl file '%s' for saving.\n" % output + str(e))
    sys.stderr.write( '\n' )
    exit(-1)

lut1d = False
size = [32, 32, 32]
lines2 = []
for i in lines:
    match = False
    m = re.search( r'TITLE\s+"([^"]+)"', i )
    if m:
        title = m.group(1)
        match = True
    m = re.search( r'DOMAIN_MIN\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)', i )
    if m:
        rgbmin[0] = float(m.group(1))
        rgbmin[1] = float(m.group(2))
        rgbmin[2] = float(m.group(3))
        match = True
    m = re.search( r'DOMAIN_MAX\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)', i )
    if m:
        rgbmax[0] = float(m.group(1))
        rgbmax[1] = float(m.group(2))
        rgbmax[2] = float(m.group(3))
        match = True
    m = re.search( r'LUT_1D_SIZE\s+(\d+)', i )
    if m:
        size = int( m.group(1) )
        lut1d = True
        match = True
    
    m = re.search( r'LUT_3D_SIZE\s+(\d+)', i )
    if m:
        size = int( m.group(1) )
        match = True
        
    if re.search( r'^#.*', i ):
        continue
    if re.search( r'^\s*$', i ):
        continue
    if match: continue
    lines2.append(i)

lines = lines2
print filename, '->', output

out.write( '// %s\n' % title )
out.write( '// CTL 3d Lut from %s\n' % filename )
out.write( '// Min: %g, %g, %g\n' % (rgbmin[0], rgbmin[1], rgbmin[2]) )
out.write( '// Max: %g, %g, %g\n' % (rgbmax[0], rgbmax[1], rgbmax[2]) )
out.write( '\n' )

if lut1d:
    out.write( '// Lut1D size %d\n' % size )
    if len(lines) != size:
        sys.stderr.write(
            'ERROR: Size of lines %d different than 1d lut size %d' %
            ( len(lines), size ) )
        size = lines.size
    out.write( '\n' )
else:
    out.write( '// Lut3D size %dx%dx%d\n' % ( size, size, size ) )
    out.write('const float min3d[3] = { %g, %g, %g };\n' %
                (rgbmin[0], rgbmin[1], rgbmin[2] ))
    out.write('const float max3d[3] = { %g, %g, %g };\n' %
                (rgbmax[0], rgbmax[1], rgbmax[2] ))
    out.write( '\n' )

    last = size * size * size
    if len(lines) != last:
        sys.stderr.write( 'ERROR: Size of lines %d different than cube size %dx%dx%d (%d)\n' %
                              ( len(lines), size, size, size, last ) )

    out.write('const float cube[%d][%d][%d][3] = \n' %
                  (size, size, size) )



if lut1d:
    r = []
    g = []
    b = []
    for x in range(0, size):
        m = re.search( r'^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)',
                           lines[x] )
        r.append( float(m.group(1)) / maxValueSpline )
        g.append( float(m.group(2)) / maxValueSpline )
        b.append( float(m.group(3)) / maxValueSpline )

    out.write( 'const float splineR[%d] = { ' )
    out.write( ', '.join(r) )
    out.write( '};\n\n' )
    
    out.write( 'const float splineG[%d] = { ' )
    out.write( ', '.join(g) )
    out.write( '};\n\n' )
    
    out.write( 'const float splineB[%d] = { ' )
    out.write( ', '.join(b) )
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
    rOut = lookup1D( splineR, #{rmin}, #{rmax}, rIn );
    gOut = lookup1D( splineG, #{gmin}, #{gmax}, gIn );
    bOut = lookup1D( splineB, #{bmin}, #{bmax}, bIn );
    aOut = aIn;
}
''')

else:
    
    idx = 0
    rgb = []

    for x in range(1, size+1):
        for y in range(1, size+1):
            for z in range(1, size+1):
                m = re.search( r'^\s*([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)\s+([-+]?[\d\.eE\+-]+)', lines[idx] )
                if m:
                    r = rgbmax[0] * float(m.group(1))
                    g = rgbmax[1] * float(m.group(2))
                    b = rgbmax[2] * float(m.group(3))
                    rgb.extend([None]*x)
                    if not rgb[x-1]: rgb[x-1] = []
                    rgb[x-1].extend([None]*y)
                    if not rgb[x-1][y-1]: rgb[x-1][y-1] = []
                    rgb[x-1][y-1].extend([None]*z)
                    rgb[x-1][y-1][z-1] = '%g, %g, %g' % (r,g,b)
                idx += 1
            
    for z in range(0, size):
        if z == 0: out.write( '{ ' )
        for y in range(0, size):
            if y == 0: out.write( '{ ' )
            for x in range(0, size):
                if x == 0: out.write( '{ ' )
                out.write( '{ %s }' % rgb[x][y][z] )
                if x != size-1:
                    out.write( ', ' )
            out.write( ' }' )
            if y != size-1:
                out.write( ', ' )
            out.write( '\n' )
        out.write( ' }' )
        if z != size-1:
            out.write( ', ' )
        out.write( '\n' )
    out.write( ' };\n\n' )

    out.write('''

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

''')
