class CreateImageCategories < ActiveRecord::Migration
  def change
    create_table :image_categories do |t|

      t.timestamps
    end
  end
end
