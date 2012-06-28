class CreateShows < ActiveRecord::Migration
  def self.up
    create_table "shows", :force => true do |t|
      t.column "name",            :string, :limit => 8,   :null => false
      t.column "title",           :string, :limit => 120
      t.column "director",        :string, :limit => 120
      t.column "production_date", :date
      t.column "delivery_date",   :date
      t.column "studio",          :string, :limit => 120
      t.column "distributor",     :string, :limit => 120
    end

    add_index "shows", ["name"], :name => "shows_name_key", :unique => true
  end

  def self.down
    drop_table "shows"
  end
end
