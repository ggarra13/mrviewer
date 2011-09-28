class Video < ActiveRecord::Base
  belongs_to :image

  IGNORE = [ 'Created at', 'Updated at' ]
  IGNORE_COLUMNS = /^(?:#{IGNORE.join('|')})$/

  TIME = [ 'Start', 'Duration' ]
  TIME_COLUMNS = /^(?:#{TIME.join('|')})$/


  validates_presence_of     :image_id
  validates_length_of       :codec,        :in => 3..30
  validates_length_of       :fourcc,       :in => 4..30
  validates_length_of       :pixel_format, :in => 4..30
  validates_numericality_of :stream,       :integer_only => true
  validates_numericality_of :fps
  validates_numericality_of :start
  validates_numericality_of :duration
  validates_uniqueness_of   :image_id, :scope => [ :stream ]

end
