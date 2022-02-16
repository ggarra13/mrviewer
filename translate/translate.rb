#!/usr/local/bin/ruby
# coding: utf-8


require "google/cloud/translate/v2"



@translate = Google::Cloud::Translate::V2.new(
  project_id: "translator-341205",
  credentials: "/home/gga/Downloads/translator-341205-98e6451016d4.json"
)


def replace( text )
  text.gsub!(/&#39;/, "'" )
  text.gsub!(/&gt;/, ">" )
  text.gsub!(/&lt;/, "<" )
  text.gsub!(/&amp;/, "&" )
  text.gsub!(/&quot;/, '"' )
  puts "result: #{text}"
  return text
end

def translate( text, lang )
  if text =~ /\//
    menus = text.split('/')
    if menus.size > 1
      puts "origin: #{text} menus"
      r = @translate.translate menus, to: lang
      result = []
      r.each { |m| result << m.text }
      if menus[-1] == '%s'
	result[-1] = '%s'
      end
      result = result.join('/')
      return replace( result )
    end
  end
  puts "origin: #{text}"
  r = @translate.translate text, to: lang
  result = r.text
  if text == ' UF: %<PRId64> ' or text == 'F: ' or text == 'T: ' or
      text == ' FC: ' or text == 'V-A: ' or
      text == ' ( %02<PRId64>:%02<PRId64>:%02<PRId64>  %d ms. )' or
      text == '  INF.  ' or
      text == 'PMem: %<PRIu64>/%<PRIu64> MB  VMem: %<PRIu64>/%<PRIu64> MB'
    result = text
  elsif text =~ /FPS:/
    result.sub!(/s*(FPS)./, 'FPS:')
  end
  if ( lang == 'ko' or lang == 'zh' or lang == "ja" ) and
      ( text =~ /mrViewer crashed\\n/ or text =~ /\\nor crushing the shadows./ )
    result.sub!(/\\/, '\n' )
  elsif lang == 'fr' and text == 'files'
    #
    # Automatic translation returns "des dossiers" which conflicts with TCLAP
    # command-line flags.  We shorten it to just dossiers.
    #
    result = 'dossiers'
  elsif lang == 'fr' and result =~ / :$/
    #
    # Automatic translation returns "des dossiers" which conflicts with TCLAP
    # command-line flags.  We shorten it to just dossiers.
    #
    result.sub!(/ :$/, ": ")
  elsif lang == 'de'
    if result =~ /\\oder/
      #
      # Automatic translation returns "\oder" instead of "\nder"
      #
      result.sub!(/\\oder/, '\nder' )
    elsif result =~ /" kann nicht gefunden werden"/
      #
      # Automatic translation returns a second line instead of just a line
      # which conflicts with \n ending in original text.
      #
      result.sub!( /" kann nicht gefunden werden"/, '' )
    end
  elsif lang == 'cs' and result =~ /\\ani/
    #
    # Automatic translation returns \an instead of \n
    #
    result.gsub!(/\\an/, '\n')
  elsif lang == 'zh' and result =~ /\\“/
    result.sub!(/\\“/, '\"' )
  elsif lang == 'ja' and result =~ /^@\s+(.*)$/
    #
    # Automatic translation returns @ || instead of @||
    #
    result = "@" + $1
  elsif lang == 'ja' and result =~ /\\s+t/
    #
    # Automatic translation returns \ instead of \n
    #
    result.gsub!(/\\s+t/, '\t')
  elsif lang == 'ja' and result =~ /\\s+"/
    #
    # Automatic translation returns \ instead of \n
    #
    result.gsub!(/\\s+"/, '\"')
  elsif lang == 'ja' and result =~ /\\[^nt]/
    #
    # Automatic translation returns \ instead of \n
    #
    result.gsub!(/\\/, '\n')
  elsif ( lang == 'zh' or lang == 'ja' ) and result =~ /（\*。{/
    result.sub!(/\s*（\*。{/, ' (*.{"')
  elsif lang == 'ko' and result == 'LM변환% 유'
    result = 'LM변환% %u'
  elsif lang == 'it' and result =~ /TEMPO LIBERO/
    result.sub!(/TEMPO LIBERO/, 'OCIO')
  elsif lang == 'de' and result =~ /FREIZEIT/
    result.sub!(/FREIZEIT/, 'OCIO')
  elsif lang == 'cs' and result =~ /VOLNÝ ČAS/
    result.sub!(/VOLNÝ ČAS/, 'OCIO')
  elsif lang == 'ko' and result =~ /여가/
    result.sub!(/여가/, 'OCIO')
  elsif lang == 'ja' and result =~ /余暇/
    result.sub!(/余暇/, 'OCIO')
  elsif lang == 'zh' and result =~ /闲暇/
    result.sub!(/闲暇/, 'OCIO')
  end
  return replace(result)
end


@h = {}

def new_line( op, text )
  return if @msgid.empty? or @h[@msgid]
  @h[@msgid] = 1
  op.puts "msgid \"#{@msgid}\""
  op.puts "msgstr \"#{text}\""
end

langs = [ 'es', 'de', 'fr', 'it', 'cs', 'zh', 'ja' ]
for lang in [ 'de', 'fr', 'it', 'cs', 'zh', 'ja', 'ko' ]
  next if langs.any? lang
  @h = {}
  puts "=================== Translate to #{lang} ======================x"
  in_msg_id = false
  msg = ''
  fp = File.open("/home/gga/code/applications/mrv/mrViewer/src/po/messages.pot", encoding: "utf-8")
  op = File.open("/home/gga/code/applications/mrv/mrViewer/src/po/#{lang}.po", "w", encoding: "utf-8")
  op.puts <<EOF
msgid ""
msgstr ""
"Project-Id-Version: mrViewer\\n"
"Report-Msgid-Bugs-To: ggarra13@gmail.com\\n"
"POT-Creation-Date: 2022-02-09 15:02-0300\\n"
"PO-Revision-Date: 2018-12-28 08:08-0300\\n"
"Last-Translator: Gonzalo Garramuño <ggarra13@gmail.com>\\n"
"Language-Team: <google.com>\\n"
"Language: #{lang}\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Generator: Poedit 2.0.6\\n"

EOF

  for line in fp
    if in_msg_id
      msgstr = line =~ /msgstr\s+"/
      text = line =~ /"(.*)"/
      if not text or msgstr
	r = translate( msg, lang )
	new_line( op, r )
	in_msg_id = false
	next
      end
      text = $1
      @msgid << text
      msg += text
      next
    end
    msgid = line =~ /msgid "(.*)"/
    if not msgid
      next
    end
    @msgid = $1
    msg = $1
    in_msg_id = true
    next
  end
  op.close
  fp.close
end
