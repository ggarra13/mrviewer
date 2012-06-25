class PixelFormat < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :image
  belongs_to :video

  has_many   :images
  has_many   :videos
end
