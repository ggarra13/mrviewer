class CreateImages < ActiveRecord::Migration
  def self.up
    create_table "images", :force => true do |t|
      t.column "directory",            :string,  :limit => 1024, :null => false
      t.column "filename",             :string,  :limit => 256, :null => false
      t.column "shot_id",              :integer
      t.column "frame_start",          :integer
      t.column "frame_end",            :integer
      t.column "creator",              :string,  :limit => 20
      t.column "created_at",           :datetime, :null => false
      t.column "updated_at",           :datetime, :null => false
      t.column "width",                :integer, :null => false
      t.column "height",               :integer, :null => false
      t.column "pixel_ratio",          :float,   :null => false, :default => 1.0
      t.column "date",                 :datetime, :null => false
      t.column "format",               :string,  :limit => 30
      t.column "fps",                  :float,   :default => 24.0
      t.column "codec",                :string,  :limit => 10    
      t.column "disk_space",           :integer, :null => false, 
      :default => 1048576
      t.column "icc_profile_id",       :integer
      t.column "render_transform_id",  :integer
      t.column "look_mod_transform_id", :integer
      t.column "depth",                :integer, :null => false, :default => 32
      t.column "pixel_format_id",    :integer
      t.column "num_channels",       :integer, :null => false, :default => 4
      t.column "layers",             :string,  :null => false, :limit => 1024
      t.column "fstop",              :float,                   :default => 8.0
      t.column "gamma",              :float,                   :default => 1.0
      t.column "online",             :boolean, :null => false, :default => true
      t.column "rating",               :integer
      t.column "backup",               :string,  :limit => 10
      t.column "label_color",          :string,  :limit => 6
      t.column "description",          :text
      t.column "thumbnail_width",      :integer, :null => false, :default => 0
      t.column "thumbnail_height",     :integer, :null => false, :default => 0
      t.column "thumbnail",            :binary
    end

    add_index "images", ["filename", "directory"], :name => "images_idx", 
    :unique => true

  end

  def self.down
    drop_table 'images'
  end
end
