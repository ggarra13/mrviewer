class ShotsController < ApplicationController
  active_scaffold :shot do |conf|
  #   conf.columns = [ :show, :sequence, :name ]   
  #   conf.actions = [ :list, :create, :show, :update, :live_search ]
  end
end
