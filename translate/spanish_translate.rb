#!/usr/bin/env ruby
# encoding: utf-8


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
  if ( text == ' UF: %<PRId64> ' or text == 'F: ' or text == 'T: ' or
       text == ' FC: ' or text == 'V-A: ' or
       text == ' ( %02<PRId64>:%02<PRId64>:%02<PRId64>  %d ms. )' or
       text == '  INF.  ' or text == "   NAN  " or
       text == 'PMem: %<PRIu64>/%<PRIu64> MB  VMem: %<PRIu64>/%<PRIu64> MB' or
       text == "mrViewer    FG: %s [%d]   BG: %s [%d] (%s)" or
       text == "mrViewer    FG: %s" or text == 'BG' or text == "EDL" or
       text == '%4.8g %%' or
       text == 'A/B' or text == 'A' or text == 'B' or
       text =~ /# Created with mrViewer/ or text == "xyY CIE xyY" or
       text == "XYZ CIE XYZ" or
       text == 'S' or text == 'E' or text == 'X' or text == 'Y' or
       text == 'X:' or text == 'Y:' or text == 'R' or text == 'G' or
       text == '15' or text == '50' or text == 'W' or text == "24000/1001" or
       text == 'B' or text == 'H' or text == 'L' or text == 'V' or
       text == "CIE xyY" or text == "CIE XYZ" or text =~ /^DYW:/ or
       text =~ /^DAW:/ or text == "FPS" )
    result = text
  elsif text =~ /FPS:/
    result.sub!(/s*(FPS)./, 'FPS:')
  end
  if ( lang == 'ar' or lang == 'cs' or lang == 'de' or lang == 'fr' or
       lang == 'gr' or lang == 'it' or lang == "ja" or lang == 'ko' or
       lang == 'nl' or lang == 'pl' or lang == 'pt' or lang == 'ro' or
       lang == 'ru' or lang == 'sv' or lang == 'tr' or lang == 'zh' ) and
      ( text =~ /mrViewer crashed\\n/ or text =~ /\\nor crushing the shadows./ )
    result.gsub!(/\\/, '\n' )
  end
  if result =~ /CPS/
    result.gsub!( /CPS/, "FPS" )
  end
  if ( lang == 'zh' or lang == 'ja' ) and result =~ /（\*。{/
    #
    # Automatic translation returns 。instead of .
    #
    result.sub!(/\s*（\*。{/, ' (*.{')
  elsif lang == 'ar'
    if text =~ /^\d+[\.:]\d+$/
      result = text
    end
    if result =~ /فراغ/
      result.gsub!( /فراغ/, 'OCIO' )
    elsif result =~ /LEISURE/
      result.gsub!( /LEISURE/, 'OCIO' )
    elsif result =~ /al'iitar/  # frame to video frame
      result.gsub!( /al'iitar/, "'iitarat" )
    end
    if result =~ /[^\\]"/
      result.gsub!( /"/, '\"' )
    elsif result =~ /^@\s+(.*)$/
      #
      # Automatic translation returns @ || instead of @||
      #
      result = "@" + $1
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /\\\s+t/
      #
      # Automatic translation returns \ t instead of \t
      #
      result.gsub!(/\\\s+t/, '\t')
    end
    if result =~ /%\s+/
      #
      # Automatic translation returns % d instead of %d or %s
      #
      result.gsub!(/%\s+/, '%')
    end
    if text == 'Saving'
      result == "تسج_ل"
    end
  elsif lang == 'cs'
    result.sub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' ) # for film aspects
    if text == 'Frame %<PRId64> '
      result = 'snímků %<PRId64> '
    elsif text == 'Reel %d (%s) | Shot %d (%s) | Frame %<PRId64> | X = %d | Y = %d\n'
      result = 'Kotouč %d (%s) | Střela %d (%s) | snímků %<PRId64> | X = %d | Y = %d\n'
    elsif text == 'Saving Sequence(s) %<PRId64> - %<PRId64>'
      result = 'Uložení sekvence(í) %<PRId64> - %<PRId64>'
    elsif text == "Save"
      result = 'Uložit'
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /VOLNÝ ČAS/
      result.sub!(/VOLNÝ ČAS/, 'OCIO')
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
    elsif result =~ /\\ani/
      #
      # Automatic translation returns \an instead of \n
      #
      result.gsub!(/\\an/, '\n')
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /\\[^n"t\\]/
      result.gsub!(/\\([^n"t\\])/, '\n\\1' )
    end
  elsif lang == 'de'
    result.sub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' ) # for film aspects
    if text == "Save"
      result = 'Speichern'
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /FREIZEIT/
      result.sub!(/FREIZEIT/, 'OCIO')
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
    elsif result =~ /\\oder/
      #
      # Automatic translation returns "\oder" instead of "\nder"
      #
      result.sub!(/\\oder/, '\nder' )
    end
    if text == 'Are you sure you want to\nremove all selected images from reel?'
      result = 'Möchten Sie wirklich alle ausgewählten\nBilder aus der Kamerarolle löschen?'
    end
    if result =~ /\\löschen/
      result.sub!( /\\löschen/, '\nlöschen' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    elsif result =~ /LAZER/
      result.gsub!( /LAZER/, 'OCIO')
    end
  elsif lang == 'es'
    if result =~ /\\\s+t/
      #
      # Automatic translation returns \ t instead of \t
      #
      result.gsub!(/\\\s+t/, '\t')
    end
  elsif lang == 'fr'
    if text == 'files'
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result = 'dossiers'
    elsif text == 'Looping Mode'
      result = 'Mode en boucle'
    elsif text == 'Saving'
      result = "Sauvegarder"
    elsif text =~ /saving/
      result.sub!( /économisez/, "sauvegarder" )
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /LOISIRS/
      result.sub!(/LOISIRS/, 'OCIO' )
    end
    if result =~ /s+i mrViewer plant\w+\s/
      result.sub!( /s+i mrViewer plant\w+\s/, ' si mrViewer plantait' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /\s:$/
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result.sub!(/\s:$/, ": ")
    end
  elsif lang == 'gr'
    if result =~ /ΕΛΕΥΘΕΡΟΣ ΧΡΟΝΟΣ/
      result.gsub!( /ΕΛΕΥΘΕΡΟΣ ΧΡΟΝΟΣ/, "OCIO" )
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
    elsif result =~ /πλαίσιο/
      result.gsub!( /πλαίσιο/, 'καρέ' )
    elsif result =~ /Πλαίσιο/
      result.gsub!( /Πλαίσιο/, 'κουτί' )
    end
    if text == 'Saving'
      result == 'αποθήκευση'
    elsif text == 'Save'
      result == 'αποθηκεύσετε'
    end
    if result =~ /[^\\]"/
      result.gsub!( /"/, '\"' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
  elsif lang == 'it'
    if text == "Saving Sequence(s) %<PRId64> - %<PRId64>"
      result = "Salvando Sequenza(e) %<PRId64> - %<PRId64>"
    end
    if result =~ /TEMPO LIBERO/
      result.sub!(/TEMPO LIBERO/, 'OCIO')
    end
    if text == '%d Hz.'
      result = text
    end
    if result =~ /\\eliminare/ or result =~ /\\cancellare/ or
      result =~ /\\sostituire/
      result.sub( /\\eliminare/, '\neliminare' )
      result.sub( /\\cancellare/, '\ncancellare' )
      result.sub( /\\sostituire/, '\nsostituire' )
    end
    if result =~ /\\[^nt"\\]/
      #
      # Automatic translation returns \ instead of \n
      #
      result.gsub!(/\\[^nt"\\]/, '\n')
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if text == 'Reel %d (%s) | Shot %d (%s) | Frame %<PRId64> | X = %d | Y = %d\n'
      result = 'Bobina %d (%s) | Colpo %d (%s) | Foto %<PRId64> | X = %d | Y = %d\n'
    end
  elsif lang == 'ja'
    if text == 'Save'
      result = "救う"
    end
    if result =~ /^([YL])（/
      result.sub!( /^([YL])（/, "\\1 （" )
    end
    if result =~ /：/
      result.gsub!(/：/, ':')
    end
    if text == "Are you sure you want to overwrite '%s'?"
      result = "'%s'を上書きしてもよろしいですか？"
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
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
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
    if text == 'Save'
      result = "저장"
    end
    if result =~ /^([YL])\(/
      result.sub!( /^([YL])\(/, "\\1 （" )
    end
    if result == 'LM변환% 유'
      result = 'LM변환 %u'
    elsif result =~ /여가/
      result.gsub!(/여가/, 'OCIO')
    elsif result =~ /LEISURE/
      result.gsub!(/LEISURE/, 'OCIO')
    end
  elsif lang == 'nl'
    if text =~ %r{^1/.*$}
      result = text
    end
    if result =~ /VRIJE TIJD/
      result.gsub!(/VRIJE TIJD/, 'OCIO')
    elsif result =~ /LEISURE/
      result.gsub!(/LEISURE/, 'OCIO')
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation sometimes returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
  elsif lang == 'pl'
    if text == 'Save'
      result = "Zapisz"
    elsif text == "Saving"
      result = "Oszczędzanie"
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /WYPOCZYNEK/
      result.gsub!( /WYPOCZYNEK/, 'OCIO' )
    elsif result =~ /leisure/
      result.gsub!( /leisure/, 'ocio' )
    end
  elsif lang == 'pt'
    if text == "Saving Sequence(s) %<PRId64> - %<PRId64>"
      result = "Guardando sequência(s) %<PRId64> - %<PRId64>"
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
  elsif lang == 'ro'
    if text == "Save"
      result = "Salvați"
    elsif text == "Saving"
      result = "Salvarea"
    elsif text == "Got colorspace '"
      result = "Am spațiul de culoare '"
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /\\să/
      result.sub!( /\\să/, '\nsă' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /AGREMENT/
      result.gsub!( /AGREMENT/, 'OCIO' )
    end
  elsif lang == 'ru'
    if text == '%d Hz.'
      result = text
    elsif text == 'W: %g %g'
      result = text
    elsif text =~ /^\d+\.\d+$/
      result = text.sub( /\./, ',' )
    end
    if result =~ /ДОСУГ/
      result.sub!(/ДОСУГ/, 'OCIO')
    elsif result =~ /ОТДЫХ/
      result.sub!(/ОТДЫХ/, 'OCIO')
    elsif result =~ /Левый "/
      result.gsub!(/"/, '\"' )
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /"\\[^n"t]/
      result.gsub!(/\\([^n"t])/, '\n\\1' )
    end
    if result =~ /"%s"/
      result.gsub!(/"/, '\"' )
    end
  elsif lang == 'sv'
    if result =~ /FRITID/
      result.sub!(/FRITID/, 'OCIO')
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
    end
    if text == 'Save'
      result = 'Spela in'
    elsif text == 'Saving'
      result = 'inspelning'
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    elsif result =~ /[^\\]"/
      result.gsub!( /"/, '\"' )
    end
  elsif lang == 'tr'
    if text == 'Save'
      result = "Kaydet"
    elsif text == "Saving"
      result == "Kaydetme"
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    end
    if result =~ /\\silmek/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\silmek/, '\nsilmek')
    end
    if result =~ /BOŞ VAKİT/
      result.gsub!( /BOŞ VAKİT/, 'OCIO' )
    end
  elsif lang == 'zh'
    if result =~ /闲暇/
      result.sub!(/闲暇/, 'OCIO')
    elsif result =~ /LEISURE/
      result.sub!(/LEISURE/, 'OCIO')
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
    if result =~ /^([YL])\s*（/
      result.sub!( /^([YL])\s*（/, "\\1 （" )
    end
    if result =~ /ID: %i n/
      result.sub!( /ID: %i n/, 'ID: %i\n' )
    elsif result =~ /id: %i n/
      result.sub!( /id: %i n/, 'id: %i\n' )
    end
    if result =~ /^\s*我从\s*$/
      result = translate( @msgid, 'zh' )
    end
  end
  # Make sure we have the same number of spaces at the end
  if text =~ /(\s+)$/
    spaces = $1
    if result !~ /#{spaces}$/
      result << spaces
    end
  end
  # Make sure we have the same number of spaces at the start
  if text =~ /^(\s+)/
    spaces = $1
    if result !~ /^#{spaces}/
      rest = result.gsub(/^s+/, '' )
      result = spaces + rest
    end
  end

  if text == 'Arabic'    and lang != 'ar' and result !~ / \(.*\)/
    result += " (عرب)"
  elsif text == 'English'    and lang != 'en' and result !~ / \(.*\)/
    result += " (English)"
  elsif text == "Spanish" and lang != 'es' and result !~ / \(.*\)/
    result += " (Español)"
  elsif text == "German"  and lang != 'de' and result !~ / \(.*\)/
    result += " (Deutsch)"
  elsif text == "Czech"   and lang != 'cs' and result !~ / \(.*\)/
    result += " (čeština)"
  elsif text == "French"  and lang != 'fr' and result !~ / \(.*\)/
    result += " (français)"
  elsif text == "Greek" and lang != 'gr' and result !~ / \(.*\)/
    result += " (Ελληνικά)"
  elsif text == "Italian" and lang != 'it' and result !~ / \(.*\)/
    result += " (italiano)"
  elsif text == "Japanese" and lang != 'ja' and result !~ / \(.*\)/
    result += " (日本)"
  elsif text == "Korean"  and lang != 'ko' and result !~ / \(.*\)/
    result += " (한국어)"
  elsif text == "Polish"  and lang != 'pl' and result !~ / \(.*\)/
    result += " (Polski)"
  elsif text == "Portuguese" and lang != 'pt' and result !~ / \(.*\)/
    result += " (português)"
  elsif text == "Romanian"   and lang != 'ro' and result !~ / \(.*\)/
    result += " (Română)"
  elsif text == "Russian"    and lang != 'ru' and result !~ / \(.*\)/
    result += " (русский)"
  elsif text == "Swedish"    and lang != 'sv' and result !~ / \(.*\)/
    result += " (svenska)"
  elsif text == "Turkish"    and lang != 'tr' and result !~ / \(.*\)/
    result += " (Türk)"
  elsif text == "Chinese"    and lang != 'zh' and result !~ / \(.*\)/
    result += " (中国人)"
  end
  return result
end

def replace( text )
  puts "result: #{text}." if @debug
  return text
end

def translate( text, lang )
  googlelang = lang
  googlelang = 'el' if lang == 'gr'
  if text =~ /\//
    menus = text.split('/')
    if menus.size > 1
      puts "#@count #{lang} spanish: #{text} menus"
      r = @translate.translate menus, from: 'es', to: googlelang
      result = []
      r.each { |m| result << m.text }
      if menus[-1] == '%s'
        result[-1] = '%s'
      end
      result = result.join('/')
      result = fix( @msgid, result, lang )
      return replace( result )
    end
  end
  puts "#@count #{lang} spanish: #{text}"
  r = @translate.translate text, from: 'es', to: googlelang
  result = r.text
  result = fix( @msgid, result, lang )
  return replace( result )
end


@debug = false
@h = {}
@op = nil

def new_line( text )
  return if @msgid.empty? or @h[@msgid]
  @h[@msgid] = 1
  @count += 1
  puts "#@count #@lang origin : #@msgid"
  @op.puts "msgid \"#{@msgid}\""
  @count += 1
  puts "#@count #@lang result : #{text}"
  @op.puts "msgstr \"#{text}\""
end



if ARGV.size > 0
  langs = ARGV
else
  langs = [ 'ar', 'cs', 'de', 'en', 'es', 'fr', 'gr', 'it', 'ja', 'ko',
            'nl', 'pl', 'pt', 'ro', 'ru', 'sv', 'tr', 'zh' ]
end


translated = [ 'es' ]
for @lang in langs
  next if translated.any? @lang
  @h = {}
  $stderr.puts "=================== Translate to #@lang ======================"
  in_msg_id = in_msg_es = false
  @msgid = ''
  root = "#{home}/gga/code/applications/mrv/mrViewer/src/po"
  fp = File.open( "#{root}/es.po", encoding: "utf-8")
  if File.exists? "#{root}/#@lang.po"
    FileUtils.cp( "#{root}/#@lang.po",
                  "#{root}/#@lang.po.old" )
  end
  @op = File.open("#{root}/#@lang.po", "w", encoding: "utf-8")
  @op.puts <<EOF
msgid ""
msgstr ""
"Project-Id-Version: mrViewer\\n"
"Report-Msgid-Bugs-To: ggarra13@gmail.com\\n"
"POT-Creation-Date: 2022-02-09 15:02-0300\\n"
"PO-Revision-Date: 2018-12-28 08:08-0300\\n"
"Last-Translator: Gonzalo Garramuño <ggarra13@gmail.com>\\n"
"Language-Team: <google.com>\\n"
"Language: #{@lang}\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Generator: Poedit 2.0.6\\n"

EOF

  msges = ''
  lines = fp.readlines
  @count = 14 # header
  for line in lines
    if in_msg_es
      text = line =~ /^"(.*)"$/
      if not text
        r = translate( msges, @lang )
        new_line( r )
        in_msg_es = in_msg_id = false
        next
      end
      msges << $1
      next
    end
    if in_msg_id
      msges = line =~ /msgstr\s+"(.*)"/
      if msges
        in_msg_es = true
        msges = $1
        next
      end
      text = line =~ /"(.*)"/
      if not text
        in_msg_id = false
        next
      end
      text = $1
      @msgid << text
      next
    end
    msgid = line =~ /msgid "(.*)"/
    if not msgid
      next
    end
    @msgid = $1
    in_msg_id = true
    next
  end
  @op.close
  fp.close
end
