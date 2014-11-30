#!/usr/bin/ruby
# -*- coding: utf-8 -*-

require 'fileutils'

license = <<EOF
/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
EOF


root = File.expand_path( File.dirname(__FILE__) )
root += '/../mrViewer/src'

Dir.foreach(root) do |dir|

  dir = "#{root}/#{dir}"

  if File.directory?(dir) and dir !~ /^\.\.?$/

    puts "Processing #{dir}..."

    Dir.foreach(dir) do |file|
      if file !~ /(?:\.cpp|\.h)$/
        next
      end


      orig = "#{dir}/#{file}"
      new  = "#{dir}/#{file}.new"

      f = File.read(orig)
      if f !~ /GNU/ 
        puts "Adding license to #{orig}"
        n = File.open(new, "w")
        n.puts license
        n.puts f
      end

      FileUtils.mv new, orig, :force => true

    end
  end
end
