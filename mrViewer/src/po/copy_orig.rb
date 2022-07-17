#!/usr/bin/env ruby

require 'fileutils'

files = Dir.glob( "*.po" )


for file in files
  FileUtils.cp_r file, "#{file}.orig"
end
