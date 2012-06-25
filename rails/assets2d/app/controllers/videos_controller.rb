class VideosController < ApplicationController
 
  active_scaffold :video do |conf|
    conf.columns = [ :thumbnail, 
                     :directory, :filename, :stream, :codec,
                     :fourcc,
                     :fps, :start, :duration  ]
    conf.actions = [ :list, :show, :update, :search ]
  end
end 
