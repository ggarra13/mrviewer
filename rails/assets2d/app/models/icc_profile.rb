class IccProfile < ActiveRecord::Base
  validates_uniqueness_of :name
  validates_uniqueness_of :filename
end
