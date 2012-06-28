class VideoCategory < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :video
  belongs_to :category

  
  validates_presence_of     :video_id
  validates_presence_of     :category_id

  def to_label
    "#{video.image.filename} #{category.name}"
  end
end
