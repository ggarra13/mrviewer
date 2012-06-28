class CreateAudios < ActiveRecord::Migration
  def self.up
    create_table "audios", :force => true do |t|
      t.column "directory",  :string,  :limit => 1024, :null => false
      t.column "filename",   :string,  :limit => 256,  :null => false
      t.column "stream",     :integer
      t.column "image_id",   :integer
      t.column "creator",    :string,  :limit => 20
      t.column "created_at", :datetime, :null => false
      t.column "updated_at", :datetime, :null => false
      t.column "disk_space", :integer,                 :default => 0
      t.column "date",       :datetime, :null => false
      t.column "shot_id",    :integer
      t.column "codec",      :string,  :null => false, :limit => 30
      t.column "fourcc",     :string,  :null => false, :limit => 30
      t.column "channels",   :integer, :null => false, :default => 1
      t.column "frequency",  :integer, :null => false
      t.column "bitrate",    :integer
      t.column "start",      :float, :null => false
      t.column "duration",   :float, :null => false
      t.column "online",     :boolean,                 :default => true
      t.column "backup",     :string,  :limit => 10
    end

    add_index "audios", ["filename", "directory", "stream"],
    :name => "audios_idx", :unique => true
  end

  def self.down
    drop_table 'audios'
  end
end
