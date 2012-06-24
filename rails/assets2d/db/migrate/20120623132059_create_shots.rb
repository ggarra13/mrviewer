class CreateShots < ActiveRecord::Migration
  def change
    create_table :shots do |t|

      t.timestamps
    end
  end
end
