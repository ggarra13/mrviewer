class CreatePixelFormats < ActiveRecord::Migration
  def self.up
    create_table "pixel_formats", :force => true do |t|
      t.column "name", :string, :limit => 256, :null => false
    end

    add_index "pixel_formats", ["name"], 
    :name => "pixel_formats_idx", :unique => true

    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('8-bits integer');
EOF
    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('16-bits integer');
EOF
    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('32-bits integer');
EOF
    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('16-bits half float');
EOF
    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('32-bits float');
EOF
    execute <<EOF
INSERT INTO pixel_formats ( name ) VALUES ('64-bits float');
EOF
  end

  def self.down
    drop_table 'pixel_formats'
  end
end
