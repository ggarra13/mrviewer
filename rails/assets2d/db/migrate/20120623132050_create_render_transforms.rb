class CreateRenderTransforms < ActiveRecord::Migration
  def change
    create_table :render_transforms do |t|

      t.timestamps
    end
  end
end
