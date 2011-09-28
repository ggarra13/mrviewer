class CreateShots < ActiveRecord::Migration
  def self.up
    create_table "shots", :force => true do |t|
      t.column "name",        :string,  :limit => 8
      t.column "sequence_id", :integer, :null => false
    end

    add_index "shots", ["sequence_id", "name"], 
    :name => "shots_sequence_id_key", :unique => true

  end

  def self.down
    drop_table "shots"
  end
end
