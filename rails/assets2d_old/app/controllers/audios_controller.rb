class AudiosController < ApplicationController

  active_scaffold :audio do |config|
    config.columns = [ :directory, :filename, :stream, :creator, :date, 
                       :disk_space, :channels, :frequency, :bitrate, 
                       :codec, :fourcc, :start, :duration, :online, :backup ]
    config.actions = [ :list, :show, :update, :live_search ]
  end

end
