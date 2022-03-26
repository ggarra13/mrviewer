#!/usr/bin/env ruby
#: encoding: utf-8

langs = [ 'cs', 'de', 'fr', 'it', 'pl', 'pt', 'ro', 'ru', 'tr' ]


for lang in langs
  puts "============ CHANGING LANGUAGE #{lang} ============="
  f = File.open( "#{lang}.po.orig", 'r' )
  o = File.open( "#{lang}.po", 'w' )
  lines = f.readlines
  f.close

  msgstr = msgid = nil
  for line in lines
    match = line =~ /^msgstr \"(\d+\.\d+)(.*)\"$/
    if not match
      o.puts line
      next
    end
    msgstr = $1
    rest   = $2
    msgstr.gsub!( /\./, ',' )
    rest.gsub!( /\./, ',' )
    puts "msgstr \"#{msgstr}#{rest}\""
    o.puts "msgstr \"#{msgstr}#{rest}\""
  end
  o.close
end
