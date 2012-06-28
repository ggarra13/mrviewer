class SequencesController < ApplicationController
  active_scaffold :sequence do |conf|
    conf.columns = [ "name", "show", "description" ]
    conf.actions = [ :list, :create, :show, :update ]
  end
end 
