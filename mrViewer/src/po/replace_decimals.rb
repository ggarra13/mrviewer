#!/usr/bin/env ruby
#: encoding: utf-8

langs = [ 'cs', 'de', 'es', 'fr', 'gr', 'it', 'nl', 'pl', 'pt',
          'ro', 'ru', 'tr' ]


for lang in langs
  puts "============ CHANGING LANGUAGE #{lang} ============="
  f = File.open( "#{lang}.po.orig", 'r' )
  o = File.open( "#{lang}.po", 'w' )
  lines = f.readlines
  f.close


  in_msg_id = false
  msgstr = msgid = nil
  for line in lines
    if not in_msg_id
      match = line =~ /^msgid \"(\d+\.\d+)(.*)\"$/
      if match
        in_msg_id = true
        msgstr = $1
      end
      o.puts line
      next
    end
    match = line =~ /^msgstr/
    if match
      in_msg_id = false
    end
    match = line =~ /^msgstr \"(\d+[\. ,]\d+)(.*)\"$/
    if match
      rest   = $2
      msgstr.gsub!( /\./, ',' )
      rest.gsub!( /\./, ',' )
      puts "msgstr \"#{msgstr}#{rest}\""
      o.puts "msgstr \"#{msgstr}#{rest}\""
    else
      o.puts line
    end
  end
  o.close
end
