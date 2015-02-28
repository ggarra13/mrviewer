class Sequence < ActiveRecord::Base
  belongs_to :show

  validates_presence_of     :show_id
  validates_length_of       :name, :within => 2..3
  validates_format_of       :name, :with => /\A\w{2,3}\z/i, 
                            :message => 'Sequence name must be like: eb or ebc' 

  validates_uniqueness_of :name, :scope => [ :show_id ], 
                                 :case_sensitive => false

  has_many  :shots
end
