class ShotsController < ApplicationController
  active_scaffold :shot do |conf|
    conf.columns = [ :sequence, :name ]   
    conf.actions = [ :list, :create, :show, :update, :search ]
  end
end
