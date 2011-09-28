class VideoCategory < ActiveRecord::Base
  validates_presence_of :video_id
  validates_presence_of :category_id

  belongs_to :category
  belongs_to :video
end
