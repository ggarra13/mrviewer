class Shot < ActiveRecord::Base

  validates_presence_of :name
  validates_presence_of :sequence_id
  validates_format_of   :name, :with => /^\w{2,3}\d{2,3}\w$/i, 
                               :message => 'Shot name must be like: ebc23 or eb023a'

  validates_uniqueness_of :name, :scope => [ :sequence_id ], 
                                 :case_sensitive => false

  belongs_to :sequence

  has_many   :images
  has_many   :audios


  def show
    if sequence_id
      sequence.show_id
    else
      nil
    end
  end

end
