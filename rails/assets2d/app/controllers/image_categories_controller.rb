class ImageCategoriesController < ApplicationController
  active_scaffold :image_category do |conf|
    conf.columns = [ :image, :category ]
    conf.actions = [ :create, :update, :list, :show ]
  end
end 
