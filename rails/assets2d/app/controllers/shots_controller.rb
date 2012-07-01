class ShotsController < ApplicationController
  active_scaffold :shot do |conf|
    conf.columns = [ :sequence, :name, :images, :audios ]   
    conf.actions = [ :list, :create, :show, :update, :search ]
  end

end
