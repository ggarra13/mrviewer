class SequencesController < ApplicationController  

  active_scaffold :sequence do |config|
    config.columns = [ :show, :name, :shots, :description ]
    config.actions = [ :list, :create, :show, :update, :live_search ]
  end

end
