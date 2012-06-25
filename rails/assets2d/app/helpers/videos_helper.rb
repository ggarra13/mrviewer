module VideosHelper

  def thumbnail_column(record)
    if record.kind_of?( Image )
      img = record
    else
      img = record.image
    end
    img.create_png
    image_tag( "dbimage-#{img.id}.png")
  end
    
  def directory_column(record)
    if record.kind_of?( Image )
      h( record.directory )
    else
      h( record.image.directory )
    end
  end
    
  def filename_column(record)
    if record.kind_of?( Image )
      h( record.filename )
    else
      h( record.image.filename )
    end
  end
    
  def pixel_format_column(record)
    h( record.pixel_format.name )
  end

  def start_column(record)
    seconds_to_time(record.start)
  end

  def duration_column(record)
    seconds_to_time(record.duration)
  end


end
