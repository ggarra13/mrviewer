class CreateLookModTransforms < ActiveRecord::Migration
  def self.up
    create_table "look_mod_transforms", :force => true do |t|
      t.column "name", :string, :limit => 256, :null => false
    end

    add_index "look_mod_transforms", ["name"], 
    :name => "look_mod_transforms_idx", :unique => true
  end

  def self.down
    drop_table 'look_mod_transforms'
  end
end
