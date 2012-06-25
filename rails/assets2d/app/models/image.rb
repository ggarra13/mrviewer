require 'RMagick'

class Image < ActiveRecord::Base

  class ThumbnailSizeError < StandardError
  end

  IGNORE = [ 'Thumbnail', 'Thumbnail width', 'Thumbnail height',
             'Created at', 'Updated at' ]
  IGNORE_COLUMNS = /^(?:#{IGNORE.join('|')})$/

  EDITABLE = [ 'Online', 'Rating', 'Backup', 'Label color', 'Description' ]

  has_many :videos
  has_many :audios
  has_one  :shot

  has_one  :icc_profile
  has_one  :render_transform
  has_one  :look_mod_transform
  has_one  :pixel_format

  belongs_to :shot
  belongs_to :icc_profile
  belongs_to :render_transform
  belongs_to :look_mod_transform
  belongs_to :pixel_format

  validates_presence_of     :directory
  validates_presence_of     :filename
  validates_presence_of     :creator
  validates_length_of       :creator, :in => 2..20
  validates_numericality_of :frame_start,  :only_integer => true
  validates_numericality_of :frame_end, :only_integer => true
  validates_numericality_of :width,  :only_integer => true
  validates_numericality_of :height, :only_integer => true
  validates_numericality_of :pixel_ratio
  validates_length_of       :format, :in => 3..30
  validates_length_of       :codec, :in => 3..10
  validates_numericality_of :disk_space, :only_integer => true
  validates_numericality_of :fps
  validates_numericality_of :fstop
  validates_numericality_of :gamma
  validates_numericality_of :rating, :only_integer => true, :allow_nil => true
  validates_format_of       :rating,
  :with    => %r{^(?:\d{1,2})?$}i,
  :message => "must be 1..10"
  validates_numericality_of :num_channels, :only_integer => true
  validates_numericality_of :depth, :only_integer => true
  validates_numericality_of :thumbnail_width,  :only_integer => true
  validates_numericality_of :thumbnail_height, :only_integer => true

  validates_uniqueness_of :filename, :scope => [ :directory ]
  validates_inclusion_of  :online, :in => [true, false]
  validates_presence_of   :backup, 
  :if => Proc.new { |image| not image.online },
  :message => "can't be blank when not online"

  class << self
    attr_accessor :thumbs
  end

  def create_png
    file = "app/assets/images/dbimage-#{id}.png"
    return if File.exist?(file) and File.ctime(file) > updated_at

    w, h = thumbnail_width, thumbnail_height
    if w <= 0 or h <= 0
      w = h = 64
      data = Array.new(w*h*4, 0.0)
    else
      data = thumbnail.unpack('C*').map { |x| x / 255.0 }
    end


    img = Magick::Image.constitute( w, h, 'BGRA', data )
    img.format = 'PNG'
    img.write(file)
  end

  def to_label
    "Image: #{filename}"
  end

end
