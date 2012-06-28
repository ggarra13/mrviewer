class IccProfile < ActiveRecord::Base
  # attr_accessible :title, :body
  has_many :images

end
