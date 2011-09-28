module ImagesHelper

  #
  # Makes the thumbnail an actual image link
  #
  def thumbnail_column(record)
    record.create_png
    image_tag( "dbimage-#{record.id}.png" )
  end
  
end
