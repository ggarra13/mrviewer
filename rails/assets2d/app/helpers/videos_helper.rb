module VideosHelper

  def thumbnail_column(record)
    record.image.create_png
    image_tag "dbimage-#{record.id}"
  end

  def directory_column(record)
    h( record.image.directory )
  end

  def filename_column(record)
    h( record.image.filename )
  end

  def start_column(record)
    seconds_to_time(record.start)
  end

  def duration_column(record)
    seconds_to_time(record.duration)
  end

end
