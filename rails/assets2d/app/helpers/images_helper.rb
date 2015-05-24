module ImagesHelper

  def pixel_format_column( record, column )
    h( record.pixel_format.name )
  end
    
  #
  # Makes the thumbnail an actual image link
  #
  def thumbnail_column( record, column )
    record.create_png
    image_tag( "dbimage#{record.id}.png" )
  end
    
  
end
