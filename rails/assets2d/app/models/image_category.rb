class ImageCategory < ActiveRecord::Base
  # attr_accessible :title, :body
  belongs_to :image
  belongs_to :category

  validates_presence_of     :image_id
  validates_presence_of     :category_id

  def to_label
    "#{image.filename} - #{category.name}"
  end

end
