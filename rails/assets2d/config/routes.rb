Rails.application.routes.draw do

  resources :video_categories do as_routes end

  resources :video_categories

  resources :videos do as_routes end

  resources :videos

  resources :users do as_routes end

  resources :users

  resources :sequences do as_routes end

  resources :sequences

  resources :shots do as_routes end

  resources :shots

  resources :render_transforms do as_routes end

  resources :render_transforms

  resources :look_mod_transforms do as_routes end

  resources :look_mod_transforms

  resources :audio_categories do as_routes end

  resources :audio_categories

  resources :categories do as_routes end

  resources :categories

  resources :audios do as_routes end

  resources :audios

  resources :image_categories do as_routes end

  resources :image_categories

  resources :pixel_formats do as_routes end

  resources :pixel_formats

  resources :icc_profiles do as_routes end

  resources :icc_profiles

  resources :images do as_routes end

  resources :images

  resources :shows do as_routes end

  resources :shows

  # The priority is based upon order of creation:
  # first created -> highest priority.

  # Sample of regular route:
  #   match 'products/:id' => 'catalog#view'
  # Keep in mind you can assign values other than :controller and :action

  # Sample of named route:
  #   match 'products/:id/purchase' => 'catalog#purchase', :as => :purchase
  # This route can be invoked with purchase_url(:id => product.id)

  # Sample resource route (maps HTTP verbs to controller actions automatically):
  #   resources :products

  # Sample resource route with options:
  #   resources :products do
  #     member do
  #       get 'short'
  #       post 'toggle'
  #     end
  #
  #     collection do
  #       get 'sold'
  #     end
  #   end

  # Sample resource route with sub-resources:
  #   resources :products do
  #     resources :comments, :sales
  #     resource :seller
  #   end

  # Sample resource route with more complex sub-resources
  #   resources :products do
  #     resources :comments
  #     resources :sales do
  #       get 'recent', :on => :collection
  #     end
  #   end

  # Sample resource route within a namespace:
  #   namespace :admin do
  #     # Directs /admin/products/* to Admin::ProductsController
  #     # (app/controllers/admin/products_controller.rb)
  #     resources :products
  #   end

  # You can have the root of your site routed with "root"
  # just remember to delete public/index.html.
  # root :to => 'welcome#index'

  # See how all your routes lay out with "rake routes"

  # This is a legacy wild controller route that's not recommended for RESTful applications.
  # Note: This route will make all actions in every controller accessible via GET requests.
  # match ':controller(/:action(/:id))(.:format)'
end
