class AudiosController < ApplicationController
  active_scaffold :audio do |conf|
    conf.columns = [ :directory, :filename, :stream, :creator, :date, 
                     :disk_space, :channels, :frequency, :bitrate, 
                     :codec, :fourcc, :start, :duration, :online, :backup ]
    conf.actions = [ :list, :show, :update, :search ]
  end
end 
