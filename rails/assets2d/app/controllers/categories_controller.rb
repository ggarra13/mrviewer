class CategoriesController < ApplicationController
  active_scaffold :category do |conf|
    conf.columns = [ :name ]
    conf.actions = [ :create, :update, :list, :show, :search ]
  end
end 
