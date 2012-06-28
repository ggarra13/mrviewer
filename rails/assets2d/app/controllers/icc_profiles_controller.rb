class IccProfilesController < ApplicationController
  active_scaffold :icc_profile do |conf|
    conf.columns = [ :name, :filename, :images ]
    conf.actions = [ :list, :create, :show, :update ]
  end
end 
