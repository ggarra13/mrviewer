class CreateAudioCategories < ActiveRecord::Migration
  def self.up
    create_table "audio_categories", :force => true do |t|
      t.column "audio_id",    :integer
      t.column "category_id", :integer
    end
  end

  def self.down
    drop_table 'audio_categories'
  end
end
