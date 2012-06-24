class CreateLookModTransforms < ActiveRecord::Migration
  def change
    create_table :look_mod_transforms do |t|

      t.timestamps
    end
  end
end
