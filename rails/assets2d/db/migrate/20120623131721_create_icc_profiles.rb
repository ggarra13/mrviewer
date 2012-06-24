class CreateIccProfiles < ActiveRecord::Migration
  def change
    create_table :icc_profiles do |t|

      t.timestamps
    end
  end
end
