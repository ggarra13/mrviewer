class AudioCategoriesController < ApplicationController
  active_scaffold :audio_category do |conf|
    conf.columns = [ :audio, :category ]
    conf.actions = [ :create, :update, :list, :show ]
  end
end 
