class Show < ActiveRecord::Base

  validates_presence_of :title, :director, :studio, :distributor

  validates_length_of     :name, :within => 3..8
  validates_uniqueness_of :name, :case_sensitive => false

  has_many :sequences
  has_many :shots,  :through => :sequences

  def validate
    if delivery_date < production_date
      errors.add(:delivery_date, 
                 "must be older than the Production Date")
    end
  end
end
