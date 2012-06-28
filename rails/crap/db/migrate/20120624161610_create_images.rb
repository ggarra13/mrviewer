class CreateImages < ActiveRecord::Migration
  def change
    create_table :images do |t|
      t.string :directory
      t.string :filename

      t.timestamps
    end
  end
end
