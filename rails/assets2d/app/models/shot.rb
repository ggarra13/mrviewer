class Shot < ActiveRecord::Base

  belongs_to :sequence


  validates_presence_of :name
  validates_presence_of :sequence_id
  validates_format_of   :name, :with => /^\w{2,3}\d{2,3}\w$/i, 
                               :message => 'Shot name must be like: ebc23 or eb023a'

  validates_uniqueness_of :name, :scope => [ :sequence_id ], 
                                 :case_sensitive => false


  has_many   :images
  has_many   :audios
  has_many   :videos, :through => :images

  def validate
    if name !~ /^#{sequence.name}/
      errors.add(:name, 
                 "must be the same prefix as the sequence")
    end
  end

  def show
    if sequence_id
      sequence.show_id
    else
      nil
    end
  end

end
