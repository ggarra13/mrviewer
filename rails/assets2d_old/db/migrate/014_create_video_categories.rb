class CreateVideoCategories < ActiveRecord::Migration
  def self.up
    create_table "video_categories", :force => true do |t|
      t.column "video_id",    :integer
      t.column "category_id", :integer
    end
  end

  def self.down
    drop_table 'video_categories'
  end
end
