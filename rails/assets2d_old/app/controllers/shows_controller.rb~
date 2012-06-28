class ShowsController < ApplicationController
  active_scaffold :show do |config|
    config.columns = [ :name, :title, :director, 
                       :production_date, :delivery_date, :studio, :distributor ]
    config.actions = [ :list, :create, :show, :update, :live_search ]
  end
end
