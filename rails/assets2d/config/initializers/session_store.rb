# Be sure to restart your server when you modify this file.

# Your secret key for verifying cookie session data integrity.
# If you change this key, all old sessions will become invalid!
# Make sure the secret is at least 30 characters and all random, 
# no regular words or you'll be exposed to dictionary attacks.
ActionController::Base.session = {
  :key         => '_assets2d_session',
  :secret      => 'a820de5348685bbb783b3809e98d5c0d7c89ae77778f850e859bd0fc4107adcaf0f690821e60c3b3e060fcae1f331f55821138cd3de1944726ece22a870a1123'
}

# Use the database for sessions instead of the cookie-based default,
# which shouldn't be used to store highly confidential information
# (create the session table with "rake db:sessions:create")
# ActionController::Base.session_store = :active_record_store
