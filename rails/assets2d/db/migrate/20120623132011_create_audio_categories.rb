class CreateAudioCategories < ActiveRecord::Migration
  def change
    create_table :audio_categories do |t|

      t.timestamps
    end
  end
end
