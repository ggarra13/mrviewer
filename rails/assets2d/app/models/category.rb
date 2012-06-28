class Category < ActiveRecord::Base
  # attr_accessible :title, :body
  has_many :image_categories
  # has_many :video_categories
  # has_many :audio_categories

  has_many :images, :through => :image_categories
  # has_many :videos, :through => :video_categories
  # has_many :audios, :through => :audio_categories
end
