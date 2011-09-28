class CreateUsers < ActiveRecord::Migration
  def self.up
    create_table "users", :force => true do |t|
      t.column :login,                     :string
      t.column :email,                     :string
      t.column :crypted_password,          :string, :limit => 40
      t.column :salt,                      :string, :limit => 40
      t.column :created_at,                :datetime
      t.column :updated_at,                :datetime
      t.column :remember_token,            :string
      t.column :remember_token_expires_at, :datetime
      t.column :created_at,                :datetime, :null => false
      t.column :updated_at,                :datetime, :null => false
    end

    a = User.new
    a.login = 'admin'
    a.email = 'webmaster@filmaura.com'
    a.password = 'fOlk32x!#'
    a.password_confirmation = a.password
    a.save!

  end

  def self.down
    drop_table "users"
  end
end
