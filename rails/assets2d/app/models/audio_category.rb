class AudioCategory < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :audio
  belongs_to :category

  validates_presence_of     :audio_id
  validates_presence_of     :category_id

  def to_label
    "#{audio.directory}/#{audio.filename} - #{category.name}"
  end
end
