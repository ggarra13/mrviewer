class CreateVideos < ActiveRecord::Migration
  def self.up
    create_table "videos", :force => true do |t|
      t.column "image_id",     :integer, :null => false
      t.column "stream",       :integer, :null => false
      t.column "created_at",           :datetime, :null => false
      t.column "updated_at",           :datetime, :null => false
      t.column "codec",        :string,  :limit => 30
      t.column "fourcc",       :string,  :limit => 30
      t.column "pixel_format", :string, :limit => 30
      t.column "fps",          :float, :null => false
      t.column "start",        :float, :null => false
      t.column "duration",     :float, :null => false
    end

    add_index "videos", ["image_id", "stream"], :name => "videos_idx", 
    :unique => true

  end

  def self.down
    drop_table 'videos'
  end
end
