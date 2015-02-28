# encoding: UTF-8
# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 15) do

  # These are extensions that must be enabled in order to support this database
  enable_extension "plpgsql"

  create_table "audio_categories", force: :cascade do |t|
    t.integer "audio_id"
    t.integer "category_id"
  end

  create_table "audios", force: :cascade do |t|
    t.string   "directory",  limit: 1024,                null: false
    t.string   "filename",   limit: 256,                 null: false
    t.integer  "stream"
    t.integer  "image_id"
    t.string   "creator",    limit: 20
    t.datetime "created_at",                             null: false
    t.datetime "updated_at",                             null: false
    t.integer  "disk_space",              default: 0
    t.datetime "date",                                   null: false
    t.integer  "shot_id"
    t.string   "codec",      limit: 30,                  null: false
    t.string   "fourcc",     limit: 30,                  null: false
    t.integer  "channels",                default: 1,    null: false
    t.integer  "frequency",                              null: false
    t.integer  "bitrate"
    t.float    "start",                                  null: false
    t.float    "duration",                               null: false
    t.boolean  "online",                  default: true
    t.string   "backup",     limit: 10
  end

  add_index "audios", ["directory", "filename", "stream"], name: "audios_idx", unique: true, using: :btree

  create_table "categories", force: :cascade do |t|
    t.string "name", limit: 1024, null: false
  end

  add_index "categories", ["name"], name: "categories_name_key", unique: true, using: :btree

  create_table "icc_profiles", force: :cascade do |t|
    t.string "name",     limit: 256
    t.string "filename", limit: 1024
  end

  add_index "icc_profiles", ["filename"], name: "icc_profiles_filename_key", unique: true, using: :btree
  add_index "icc_profiles", ["name"], name: "icc_profiles_name_key", unique: true, using: :btree

  create_table "image_categories", force: :cascade do |t|
    t.integer "image_id"
    t.integer "category_id"
  end

  create_table "images", force: :cascade do |t|
    t.string   "directory",           limit: 1024,                   null: false
    t.string   "filename",            limit: 256,                    null: false
    t.integer  "shot_id"
    t.integer  "frame_start"
    t.integer  "frame_end"
    t.string   "creator",             limit: 20
    t.datetime "created_at",                                         null: false
    t.datetime "updated_at",                                         null: false
    t.integer  "width",                                              null: false
    t.integer  "height",                                             null: false
    t.float    "pixel_ratio",                      default: 1.0,     null: false
    t.datetime "date",                                               null: false
    t.string   "format",              limit: 30
    t.float    "fps",                              default: 24.0
    t.string   "codec",               limit: 10
    t.integer  "disk_space",                       default: 1048576, null: false
    t.integer  "icc_profile_id"
    t.integer  "render_transform_id"
    t.integer  "look_mod_image_id"
    t.integer  "depth",                            default: 32,      null: false
    t.integer  "pixel_format_id"
    t.integer  "num_channels",                     default: 4,       null: false
    t.string   "layers",              limit: 1024,                   null: false
    t.float    "fstop",                            default: 8.0
    t.float    "gamma",                            default: 1.0
    t.boolean  "online",                           default: true,    null: false
    t.integer  "rating"
    t.string   "backup",              limit: 10
    t.string   "label_color",         limit: 6
    t.string   "description",         limit: 1024
    t.integer  "thumbnail_width",                  default: 0,       null: false
    t.integer  "thumbnail_height",                 default: 0,       null: false
    t.binary   "thumbnail"
  end

  add_index "images", ["directory", "filename"], name: "images_idx", unique: true, using: :btree

  create_table "look_mod_images", force: :cascade do |t|
    t.integer "image_id"
    t.integer "look_mod_transform_id"
  end

  create_table "look_mod_transforms", force: :cascade do |t|
    t.string "name", limit: 256, null: false
  end

  add_index "look_mod_transforms", ["name"], name: "look_mod_transforms_idx", unique: true, using: :btree

  create_table "pixel_formats", force: :cascade do |t|
    t.string "name", limit: 256, null: false
  end

  add_index "pixel_formats", ["name"], name: "pixel_formats_idx", unique: true, using: :btree

  create_table "render_transforms", force: :cascade do |t|
    t.string "name", limit: 256, null: false
  end

  add_index "render_transforms", ["name"], name: "render_transforms_idx", unique: true, using: :btree

  create_table "sequences", force: :cascade do |t|
    t.string  "name",        limit: 4
    t.integer "show_id",               null: false
    t.text    "description"
  end

  add_index "sequences", ["name", "show_id"], name: "sequences_show_id_key", unique: true, using: :btree

  create_table "shots", force: :cascade do |t|
    t.string  "name",        limit: 8
    t.integer "sequence_id",           null: false
  end

  add_index "shots", ["name", "sequence_id"], name: "shots_sequence_id_key", unique: true, using: :btree

  create_table "shows", force: :cascade do |t|
    t.string "name",            limit: 8,   null: false
    t.string "title",           limit: 120
    t.string "director",        limit: 120
    t.date   "production_date"
    t.date   "delivery_date"
    t.string "studio",          limit: 120
    t.string "distributor",     limit: 120
  end

  add_index "shows", ["name"], name: "shows_name_key", unique: true, using: :btree

  create_table "users", force: :cascade do |t|
    t.string   "login",                     limit: 255
    t.string   "email",                     limit: 255
    t.string   "crypted_password",          limit: 40
    t.string   "salt",                      limit: 40
    t.datetime "created_at",                            null: false
    t.datetime "updated_at",                            null: false
    t.string   "remember_token",            limit: 255
    t.datetime "remember_token_expires_at"
  end

  create_table "video_categories", force: :cascade do |t|
    t.integer "video_id"
    t.integer "category_id"
  end

  create_table "videos", force: :cascade do |t|
    t.integer  "image_id",                   null: false
    t.integer  "stream",                     null: false
    t.datetime "created_at",                 null: false
    t.datetime "updated_at",                 null: false
    t.string   "codec",           limit: 30
    t.string   "fourcc",          limit: 30
    t.integer  "pixel_format_id"
    t.float    "fps",                        null: false
    t.float    "start",                      null: false
    t.float    "duration",                   null: false
  end

  add_index "videos", ["image_id", "stream"], name: "videos_idx", unique: true, using: :btree

end
