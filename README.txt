
Compilation of mrViewer requires the following libraries:

- Boost       1.42		DONE
- SampleICC   1.6.4             mingw cross compiled
- OpenEXR     1.6.1             DONE
- Zlib        1.2.5		DONE
- IlmBase     1.0.1             DONE
- CTL         1.4.1		DONE
- OpenEXR CTL 1.0.1 ( with ImfCtlApplyTransforms )
- ImageMagick 6.6.8-10          DONE precompiled
- TCLAP       1.2.0             DONE
- FLTK        2.0.x  HEAD       DONE
- FFMPEG      HEAD              DONE precompiled
- PostgresSQL                   DONE precompiled
- Gettext     1.5?              DONE
- OpenGL      1.4+              DONE
- GLEW        1.7               DONE precompiled
- GLUT        1.2               DONE
- CMake       2.8               DONE

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
