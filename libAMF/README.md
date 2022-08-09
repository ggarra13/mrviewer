# libAMF
Library for reading and writing AMF clip-level Metadata File Format

The library consists of a reader and a writer.  They currently operate on all header information and provide support for IDT, LMTs, RRT and ODT.  Each transform has a status indicating if it is active or not.
The library makes no attempt to keep the unparsed data in the xml file in the class.
