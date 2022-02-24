#!/usr/bin/env ruby
# coding: utf-8


require "google/cloud/translate/v2"
require "fileutils"

home = '/home'
if RUBY_PLATFORM =~ /Darwin/i
  home = '/Users'
  cred = '/Users/gga/Downloads/translator-341205-b03ffc77569a.json'
else
  cred = '/home/gga/Downloads/translator-341205-98e6451016d4.json'
end

@translate = Google::Cloud::Translate::V2.new(
  project_id: "translator-341205",
  credentials: "#{cred}"
)


def fix( text, result, lang )
  result.gsub!(/&#39;/, "'" )
  result.gsub!(/&gt;/, ">" )
  result.gsub!(/&lt;/, "<" )
  result.gsub!(/&amp;/, "&" )
  result.gsub!(/&quot;/, '"' )
  if text == ' UF: %<PRId64> ' or text == 'F: ' or text == 'T: ' or
      text == ' FC: ' or text == 'V-A: ' or
      text == ' ( %02<PRId64>:%02<PRId64>:%02<PRId64>  %d ms. )' or
      text == '  INF.  ' or
      text == 'PMem: %<PRIu64>/%<PRIu64> MB  VMem: %<PRIu64>/%<PRIu64> MB' or
      text == "mrViewer    FG: %s [%d]   BG: %s [%d] (%s)" or
      text == "mrViewer    FG: %s" or
      text =~ /# Created with mrViewer/
    result = text
  elsif text =~ /FPS:/
    result.sub!(/s*(FPS)./, 'FPS:')
  end
  if ( lang == 'ko' or lang == 'zh' or lang == "ja" or lang == 'ru' or
       lang == 'tr' ) and
      ( text =~ /mrViewer crashed\\n/ or text =~ /\\nor crushing the shadows./ )
    result.gsub!(/\\/, '\n' )
  elsif ( lang == 'zh' or lang == 'ja' ) and result =~ /（\*。{/
    #
    # Automatic translation returns 。instead of .
    #
    result.sub!(/\s*（\*。{/, ' (*.{')
  elsif lang == 'fr'
    if text == 'files'
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result = 'dossiers'
    elsif result =~ /LOISIRS/
      result.sub!(/LOISIRS/, 'OCIO' )
    end
    if result =~ / :$/
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result.sub!(/\s:$/, ": ")
    end
  elsif lang == 'de'
    if text == '%4.8g %%'
      result = text
    end
    if result =~ /,* kann nicht gefunden werden/
      #
      # Automatic translation returns a second line instead of just a line
      # which conflicts with \n ending in original text.
      #
      result.sub!( /,? kann nicht gefunden werden/, '' )
    elsif result =~ /FREIZEIT/
      result.sub!(/FREIZEIT/, 'OCIO')
    elsif result =~ /\\oder/
      #
      # Automatic translation returns "\oder" instead of "\nder"
      #
      result.sub!(/\\oder/, '\nder' )
    end
  elsif lang == 'cs'
    if text == '%4.8g %%'
      result = text
    end
    if text == 'Frame %<PRId64> '
      result = 'snímků %<PRId64> '
    end
    if text == 'Reel %d (%s) | Shot %d (%s) | Frame %<PRId64> | X = %d | Y = %d\n'
      result = 'Kotouč %d (%s) | Střela %d (%s) | snímků %<PRId64> | X = %d | Y = %d\n'
    end
    if text == "Saving Sequence(s) %<PRId64> - %<PRId64>"
      result = 'Ukládání sekvencí %<PRId64> - %<PRId64>'
    end
    if result =~ /VOLNÝ ČAS/
      result.sub!(/VOLNÝ ČAS/, 'OCIO')
    elsif result =~ /\\ani/
      #
      # Automatic translation returns \an instead of \n
      #
      result.gsub!(/\\an/, '\n')
    end
  elsif lang == 'ru'
    if text == '%4.8g %%'
      result = text
    end
    if text == '%d Hz.'
      result = text
    end
    if text == 'W: %g %g'
      result = 'B: %g %g'
    end
    if result =~ /ДОСУГ/
      result.sub!(/ДОСУГ/, 'OCIO')
    elsif result =~ /ОТДЫХ/
      result.sub!(/ОТДЫХ/, 'OCIO')
    elsif result =~ /Левый "/
      result.gsub!(/"/, '\"' )
    end
    if result =~ /"\\[^n"t]/
      result.gsub!(/\\[^n"t]/, '\n' )
    end
    if result =~ /"%s"/
      result.gsub!(/"/, '\"' )
    end
  elsif lang == 'ja'
    if result =~ /：/
      result.gsub!(/：/, ':')
    end
    if text =~ /:(\s+)/
      spaces = $1
      if result !~ /:#{spaces}/
        result.gsub!( /:\s*/, ":#{spaces}" )
      end
    end
    if text == 'LMTransform %u'
      result = text
    end
    if result =~ /余暇/
      result.sub!(/余暇/, 'OCIO')
    elsif result =~ /^@\s+(.*)$/
      #
      # Automatic translation returns @ || instead of @||
      #
      result = "@" + $1
    end
    if result =~ /。/
      result.gsub!(/。/, '.')
    end
    if result =~ /％/
      result.gsub!(/％/, '%' )
    end
    if result =~ /(\s*)％\s*([dg])(\s*)/
      result.gsub!(/(\s*)％\s*([dg])(\s*)/, $1 + '%' + $2 + $3 )
    end
    if result =~ /\\\s+"/
      #
      # Automatic translation returns \ " instead of \"
      #
      result.gsub!(/\\\s+"/, '\\"')
    end
    if result =~ /\\\s+t/
      #
      # Automatic translation returns \ t instead of \t
      #
      result.gsub!(/\\\s+t/, '\t')
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /\\\s+[^nt"\\]/
      #
      # Automatic translation returns \ instead of \n
      #
      result.gsub!(/\\\s+/, '\n')
    end
    if text == 'W: %g %g'
      result = text
    end
  elsif lang == 'ko'
    if result == 'LM변환% 유'
      result = 'LM변환 %u'
    elsif result =~ /여가/
      result.sub!(/여가/, 'OCIO')
    end
  elsif lang == 'it'
    if result =~ /TEMPO LIBERO/
      result.sub!(/TEMPO LIBERO/, 'OCIO')
    end
    if text == '%4.8g %%'
      result = text
    end
    if text == 'Reel %d (%s) | Shot %d (%s) | Frame %<PRId64> | X = %d | Y = %d\n'
      result = 'Bobina %d (%s) | Colpo %d (%s) | Foto %<PRId64> | X = %d | S = %d\n'
    end
  elsif lang == 'zh'
    if text == '%4.8g %%'
      result = text
    end
    if result =~ /闲暇/
      result.sub!(/闲暇/, 'OCIO')
    end
    t = '[“”]'
    if result =~ /#{t}/
      result.gsub!( /#{t}/, '"' )
      result.gsub!( /\\/, '' )
      result.gsub!( /"/, '\"' )
    end
    if result =~ /％/
      result.gsub!( /％/, '%' )
    end
    if result =~ /ID: %in/
      result.sub!( /ID: %in/, 'ID: %i\n' )
    end
  end
  if text =~ /(\s+)$/
    spaces = $1
    if result !~ /#{spaces}$/
      result << spaces
    end
  end
  if text =~ /^(\s+)/
    spaces = $1
    if result !~ /^#{spaces}/
      text = result.gsub(/^s+/, '' )
      result = spaces + text
    end
  end
  return result
end

def replace( text )
  puts "result: #{text}." if @debug
  return text
end

def translate( text, lang )
  if text =~ /\//
    menus = text.split('/')
    if menus.size > 1
      puts "#@count origin: #{text} menus" if @debug
      r = @translate.translate menus, from: 'en', to: lang
      result = []
      r.each { |m| result << m.text }
      if menus[-1] == '%s'
        result[-1] = '%s'
      end
      result = result.join('/')
      result = fix( text, result, lang )
      return replace( result )
    end
  end
  puts "#@count origin: #{text}" if @debug
  r = @translate.translate text, from: 'en', to: lang
  result = r.text
  result = fix( text, result, lang )
  return replace( result )
end


@debug = false
@h = {}
@op = nil

def new_line( text )
  return if @msgid.empty? or @h[@msgid]
  @h[@msgid] = 1
  @count += 1
  puts "#@count origin: #@msgid"
  @op.puts "msgid \"#{@msgid}\""
  @count += 1
  puts "#@count result: #{text}"
  @op.puts "msgstr \"#{text}\""
end

langs = [ 'es' ]
for lang in [ 'de', 'fr', 'it', 'cs', 'ru', 'zh', 'ja', 'ko' ]
  next if langs.any? lang
  @h = {}
  $stderr.puts "=================== Translate to #{lang} ======================"
  in_msg_id = false
  msg = ''
  root = "#{home}/gga/code/applications/mrv/mrViewer/src/po"
  fp = File.open( "#{root}/messages.pot", encoding: "utf-8")
  if File.exists? "#{root}/#{lang}.po"
    FileUtils.cp( "#{root}/#{lang}.po",
                  "#{root}/#{lang}.po.old" )
  end
  @op = File.open("#{root}/#{lang}.po", "w", encoding: "utf-8")
  @op.puts <<EOF
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

  lines = fp.readlines
  @count = 14 # header
  for line in lines
    if in_msg_id
      msgstr = line =~ /msgstr\s+"/
      text = line =~ /"(.*)"/
      if not text or msgstr
        r = translate( msg, lang )
        new_line( r )
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
  @op.close
  fp.close
end
