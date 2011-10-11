
Compilation of mrViewer requires the following libraries:

- Boost       1.42
- SampleICC   1.6.4
- OpenEXR     1.6.1
- CTL         1.4.1
- OpenEXR CTL 1.0.1 ( with ImfCtlApplyTransforms )
- ImageMagick 6.6.8-10
- TCLAP       1.2.0
- FLTK        2.0.x HEAD
- FFMPEG      HEAD
- PostgresSQL 
- Gettext
- OpenGL      1.4+
- GLUT        1.2

Compilation is started with 'mk', a bash command which calls cmake in 
the process and later make.  
Compilation goes into BUILD/{os}-{version}/Release/bin

For compiling some non opengl shaders, Cg is required.

To install the viewer for best performancce, both the shaders and ctl 
directories must be present under a single root, like:

Release/
   	bin/
		mrViewer
	ctl/
		transform_RRT.ctl
	shaders/
		rgba.cg
