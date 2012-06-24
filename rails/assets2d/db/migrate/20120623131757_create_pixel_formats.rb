class CreatePixelFormats < ActiveRecord::Migration
  def change
    create_table :pixel_formats do |t|

      t.timestamps
    end
  end
end
