class AudioCategory < ActiveRecord::Base
  validates_presence_of :audio_id
  validates_presence_of :category_id

  belongs_to :category
  belongs_to :audio
end
