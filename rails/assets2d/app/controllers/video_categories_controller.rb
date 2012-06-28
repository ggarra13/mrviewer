class VideoCategoriesController < ApplicationController
  active_scaffold :video_category do |conf|
    conf.columns = [ :video, :category ]
    conf.actions = [ :create, :update, :list, :show ]
  end
end 
