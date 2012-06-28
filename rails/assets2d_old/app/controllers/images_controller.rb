class ImagesController < ApplicationController
  active_scaffold :image do |config|
    config.columns = [ :thumbnail, :directory, :filename, 
                       :frame_start, :frame_end,
                       :creator, :width, :height, :pixel_ratio, :date,
                       :format, :fps, :codec, :disk_space, :depth, 
                       :num_channels, :layers, :fstop, :gamma, :rating, 
                       :online, :backup, :description ]
    config.actions = [ :list, :show, :update, :search ]
  end
end
