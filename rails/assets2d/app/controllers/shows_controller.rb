class ShowsController < ApplicationController
  active_scaffold :show do |conf|
    conf.columns = [ :name, :title, :director, 
                     :production_date, :delivery_date, :studio, :distributor ]
    conf.actions = [ :list, :create, :show, :update ]
  end

end 
