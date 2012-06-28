class CreateCategories < ActiveRecord::Migration
  def self.up
    create_table "categories", :force => true do |t|
      t.column "name", :string, :limit => 1024, :null => false
    end

    add_index "categories", ["name"], 
    :name => "categories_name_key", :unique => true
  end

  def self.down
    drop_table 'categories'
  end
end
