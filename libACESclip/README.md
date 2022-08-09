# libACESclip
Library for reading and writing ACES clip-level Metadata File Format

The library consists of a reader and a writer.  They currently operate on all header information and provide support for IDT, LMTs, RRT and ODT.  Each transform has a status indicating if it is active (ACES::kPreview) or not (ACES::kApplied).  In addition to those transforms, the linkInputTransform and the linkPreviewTransform are also read if present and can be saved too.
The library makes no attempt to keep the unparsed data in the xml file in the class.
