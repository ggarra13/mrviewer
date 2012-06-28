class CreateIccProfiles < ActiveRecord::Migration
  def self.up
    create_table "icc_profiles", :force => true do |t|
      t.column "name",     :string, :limit => 256
      t.column "filename", :string, :limit => 1024
    end

    add_index "icc_profiles", ["name"], 
    :name => "icc_profiles_name_key", :unique => true

    add_index "icc_profiles", ["filename"], 
    :name => "icc_profiles_filename_key", :unique => true
  end

  def self.down
    drop_table 'icc_profiles'
  end
end
