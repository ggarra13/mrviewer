

// dwab_experiment
//
// Program to write out a sequence of DWAB OpenEXR files with
// different compression levels.
//
// by Brendan Bolles <brendan@fnordware.com>
//

#include <ImfInputFile.h>
#include <ImfChannelList.h>

#include <ImfOutputFile.h>
#include <ImfStandardAttributes.h>

#include <ImfStdIO.h>

#include <IlmThread.h>
#include <IlmThreadPool.h>

#include <iostream>
#include <unistd.h>


using namespace Imf;
using namespace Imath;
using namespace std;



template <typename T>
class AutoArray
{
  public:
	AutoArray() : _ptr(NULL) {}
	AutoArray(T *p) : _ptr(p) {}
	~AutoArray()
	{
		if(_ptr)
			delete [] _ptr;
	}
	
	AutoArray & operator = (T *p) { _ptr = p; return *this; }
	
	operator T * () const { return _ptr; }
	T * get() const { return _ptr; }

  private:
	T *_ptr;
};


int main (int argc, char * const argv[])
{
	
	if(argc != 3)
	{
		cout << "Usage: " << argv[0] << " /path/to/input.exr /path/to/output.%04d.exr" << endl;
		return -1;
	}
	
	try{
	
	if( IlmThread::supportsThreads() )
		setGlobalThreadCount( sysconf(_SC_NPROCESSORS_ONLN) );
	
	
	InputFile input_file( argv[1] );
	
	
	const Header &head = input_file.header();
	
	const Box2i &dataW = head.dataWindow();
	const Box2i &dispW = head.displayWindow();
	
	const int width = dataW.max.x - dataW.min.x + 1;
	const int height = dataW.max.y - dataW.min.y + 1;
	
	
	AutoArray<half> rgba_buf = new half[width * height * 4];
	AutoArray<float> z_buf = new float[width * height];
	
	
	const size_t rowbytes = sizeof(half) * width * 4;
	char *exr_RGBA_origin = (char *)rgba_buf.get() - (sizeof(half) * dataW.min.x) - (rowbytes * dataW.min.y);
	
	const size_t z_rowbytes = sizeof(float) * width;
	char *exr_z_origin = (char *)z_buf.get() - (sizeof(float) * dataW.min.x) - (z_rowbytes * dataW.min.y);
	
	
	FrameBuffer frameBuffer;
	
	frameBuffer.insert("R", Slice(Imf::HALF, exr_RGBA_origin + (sizeof(half) * 0), sizeof(half) * 4, rowbytes, 1, 1, 0.0) );
	frameBuffer.insert("G", Slice(Imf::HALF, exr_RGBA_origin + (sizeof(half) * 1), sizeof(half) * 4, rowbytes, 1, 1, 0.0) );
	frameBuffer.insert("B", Slice(Imf::HALF, exr_RGBA_origin + (sizeof(half) * 2), sizeof(half) * 4, rowbytes, 1, 1, 0.0) );
	frameBuffer.insert("A", Slice(Imf::HALF, exr_RGBA_origin + (sizeof(half) * 3), sizeof(half) * 4, rowbytes, 1, 1, 1.0) );
	frameBuffer.insert("Z", Slice(Imf::FLOAT, exr_z_origin + (sizeof(float) * 0), sizeof(float) * 1, z_rowbytes, 1, 1, 0.0) );
	
	
	input_file.setFrameBuffer(frameBuffer);
	
	input_file.readPixels(dataW.min.y, dataW.max.y);
	
	
	
	for(int frame=0; frame < 150; frame++)
	{
		Header output_header(dispW, dataW, 1, V2f(0, 0), 1, INCREASING_Y, DWAB_COMPRESSION);
		
		output_header.channels().insert("R", Channel(Imf::HALF));
		output_header.channels().insert("G", Channel(Imf::HALF));
		output_header.channels().insert("B", Channel(Imf::HALF));
		
		if( head.channels().findChannel("A") )
			output_header.channels().insert("A", Channel(Imf::HALF));
		
		if( head.channels().findChannel("Z") )
			output_header.channels().insert("Z", Channel(Imf::FLOAT));
			
		
		const float compressionLevel = (frame == 0 ? 0 :
										frame == 41 ? 45.0 : // the default value
										pow(10.0, frame / 25.0));
		
		addDwaCompressionLevel(output_header, compressionLevel);
		
		
		char file_path[256];
		
		snprintf(file_path, 255, argv[2], frame);
		
		
		StdOFStream output_stream(file_path);
		OutputFile output_file(output_stream, output_header);
		
		output_file.setFrameBuffer(frameBuffer);
		
		output_file.writePixels(height);
		
		
		cout << "frame:\t" << frame << "\tcompression level:\t" << compressionLevel;
		cout << "\tsize:\t" << (output_stream.tellp() / 1024) << endl;
	}
	
	}catch(...) { return -1; }
	
    return 0;
}
