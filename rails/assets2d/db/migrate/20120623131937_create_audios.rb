class CreateAudios < ActiveRecord::Migration
  def change
    create_table :audios do |t|

      t.timestamps
    end
  end
end
