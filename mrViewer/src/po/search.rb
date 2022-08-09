#!/usr/bin/env ruby
#: encoding: utf-8


f = File.open( "es.po", 'r' )
lines = f.readlines
f.close

msgstr = false
lineno = oldlineno = 0
for line in lines
  lineno += 1
  if msgstr and line =~ /^$/
    if oldlineno == lineno-1
      puts lineno
    end
  end

  match = line =~ /^msgstr \"\"$/
  next if not match
  if match
    msgstr = true
    oldlineno = lineno
  end
end
