module VideosHelper

  def thumbnail_column(record)
    record.create_png
    image_tag( "dbimage-#{record.id}.png")
  end

  def directory_column(record)
    h( record.directory )
  end

  def filename_column(record)
    h( record.filename )
  end

  def start_column(record)
    seconds_to_time(record.start)
  end

  def duration_column(record)
    seconds_to_time(record.duration)
  end

end
