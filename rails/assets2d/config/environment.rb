# Force rails into production mode
ENV['RAILS_ENV'] ||= 'production'

# Load the Rails application.
require File.expand_path('../application', __FILE__)

# Initialize the Rails application.
Rails.application.initialize!
