class Audio < ActiveRecord::Base

  TIME = [ 'Start', 'Duration' ]
  TIME_COLUMNS = /^(?:#{TIME.join('|')})$/

  IGNORE = [ 'Created at', 'Updated at' ]
  IGNORE_COLUMNS = /^(?:#{IGNORE.join('|')})$/

  EDITABLE = [ 'Online', 'Rating', 'Backup', 'Label color', 'Description' ]

  
  validates_presence_of     :directory
  validates_presence_of     :filename
  validates_presence_of     :date
  validates_presence_of     :creator
  validates_length_of       :creator,      :in => 2..20
  validates_length_of       :codec,        :in => 3..30
  validates_length_of       :fourcc,       :in => 4..30
  validates_length_of       :pixel_format, :in => 4..30
  validates_numericality_of :stream,       :integer_only => true
  validates_numericality_of :fps
  validates_numericality_of :start
  validates_numericality_of :duration
  validates_numericality_of :disk_space,   :integer_only => true
  validates_numericality_of :channels,     :integer_only => true
  validates_numericality_of :frequency,    :integer_only => true
  validates_numericality_of :bitrate,      :integer_only => true
  validates_uniqueness_of   :filename, :scope => [ :directory, :stream ]

  has_one :image

end
