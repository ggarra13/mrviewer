class CreateSequences < ActiveRecord::Migration
  def self.up

    create_table "sequences", :force => true do |t|
      t.column "name",        :string,  :limit => 4
      t.column "show_id",     :integer,              :null => false
      t.column "description", :text
    end

    add_index "sequences", ["show_id", "name"], 
    :name => "sequences_show_id_key", :unique => true
  end

  def self.down
    drop_table "sequences"
  end
end
