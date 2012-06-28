class VideosController < ApplicationController

  active_scaffold :video do |config|
    config.columns = [ :thumbnail, 
                       :directory, :filename, :stream, :codec, :fourcc,
                       :pixel_format, :fps, :start, :duration  ]
    config.actions = [ :list, :show, :update, :live_search ]
  end

end
