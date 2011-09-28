class CreateImageCategories < ActiveRecord::Migration
  def self.up
    create_table "image_categories", :force => true do |t|
      t.column "image_id",    :integer
      t.column "category_id", :integer
    end
  end

  def self.down
    drop_table 'image_categories'
  end
end
