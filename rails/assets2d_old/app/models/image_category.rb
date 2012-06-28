class ImageCategory < ActiveRecord::Base
  validates_presence_of :image_id
  validates_presence_of :category_id

  belongs_to :category
  belongs_to :image
end
