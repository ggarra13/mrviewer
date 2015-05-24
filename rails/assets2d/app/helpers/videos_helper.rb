module VideosHelper

  def thumbnail_column(record, column)
    if record.kind_of?( Image )
      img = record
    else
      img = record.image
    end
    img.create_png
    image_tag( "dbimage#{img.id}.png")
  end
    
  def directory_column(record, column)
    if record.kind_of?( Video )
      h( record.image.directory )
    else
      h( record.directory )
    end
  end
    
  def filename_column(record, column)
    if record.kind_of?( Video )
      h( record.image.filename )
    else
      h( record.filename )
    end
  end
    
  def pixel_format_column(record, column)
    h( record.pixel_format.name )
  end

  def start_column(record, column)
    seconds_to_time(record.start)
  end

  def duration_column(record, column)
    seconds_to_time(record.duration)
  end


end
