# Filters added to this controller apply to all controllers in the application.
# Likewise, all the methods added will be available for all controllers.

class ApplicationController < ActionController::Base
  layout 'assets2d'

  # Pick a unique cookie name to distinguish our session data from others'
  session :session_key => '_assets2d_session_id'
end
