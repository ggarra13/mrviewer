class ShotsController < ApplicationController
  active_scaffold :shot do |config|
    config.columns = [ :show, :sequence, :name ]   
    config.actions = [ :list, :create, :show, :update, :live_search ]
  end
end
