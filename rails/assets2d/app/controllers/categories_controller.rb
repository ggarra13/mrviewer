class CategoriesController < ApplicationController

  active_scaffold :categories do |config|
    config.columns = [ :name, :images, :videos, :audios ]
    config.actions = [ :list, :show, :create, :update, :delete, :live_search ]
  end

end
