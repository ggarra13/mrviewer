class CreateRenderTransforms < ActiveRecord::Migration
  def self.up
    create_table "render_transforms", :force => true do |t|
      t.column "name", :string, :limit => 256, :null => false
    end

    add_index "render_transforms", ["name"], 
    :name => "render_transforms_idx", :unique => true
  end

  def self.down
    drop_table 'render_transforms'
  end
end
