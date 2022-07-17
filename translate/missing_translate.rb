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


def fix( text, result )
  result.gsub!(/&#39;/, "'" )
  result.gsub!(/&gt;/, ">" )
  result.gsub!(/&lt;/, "<" )
  result.gsub!(/&amp;/, "&" )
  result.gsub!(/&quot;/, '"' )
  if text == ' UF: %<PRId64> ' or text == 'F: ' or text == 'T: ' or
    text == ' FC: ' or text == 'V-A: ' or
    text == ' ( %02<PRId64>:%02<PRId64>:%02<PRId64>  %d ms. )' or
    text == '  INF.  ' or text == "   NAN  " or
    text == 'PMem: %<PRIu64>/%<PRIu64> MB  VMem: %<PRIu64>/%<PRIu64> MB' or
    text == "mrViewer    FG: %s [%d]   BG: %s [%d] (%s)" or
    text == "mrViewer    FG: %s" or text == '%4.8g %%' or
    text == 'A' or text == 'A/B' or text == "FPS" or
    text =~ /# Created with mrViewer/
    result = text
  elsif text =~ /FPS:/
    result.sub!(/s*(FPS)./, 'FPS:')
  end
  if ( @lang == 'ko' or @lang == 'zh' or @lang == "ja" or @lang == 'ru' or
       @lang == 'tr' or @lang == 'pt'or @lang == 'ro' or @lang == 'pl') and
      ( text =~ /mrViewer crashed\\n/ or text =~ /\\nor crushing the shadows./ )
    result.gsub!(/\\/, '\n' )
  elsif (@lang == 'zh' or @lang == 'ja' ) and result =~ /（\*。{/
    #
    # Automatic translation returns 。instead of .
    #
    result.sub!(/\s*（\*。{/, ' (*.{')
  elsif @lang == 'cs'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
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
    elsif result =~ /Ukládání/ or result =~ /Úspora/
      result.gsub!( /Ukládání/, 'Záznam' )
      result.gsub!( /Úspora/, 'Záznam' )
    elsif result =~ /ukládání/
      result.gsub!( /ukládání/, 'záznam' )
    end
  elsif @lang == 'de'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
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
  elsif @lang == 'es'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
    if result =~ /\\\s+t/
      #
      # Automatic translation returns \ t instead of \t
      #
      result.gsub!(/\\\s+t/, '\t')
    end
  elsif @lang == 'fr'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
    if text == 'files'
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result = 'dossiers'
    elsif text == "A/B"
      result = text
    elsif text == "A-B, Stereo 3D Options"
      result = "A-B, options 3D stéréo"
    elsif text == "OpenEXR"
      result = text
    elsif result =~ /LOISIRS/
      result.sub!(/LOISIRS/, 'OCIO' )
    elsif result =~ /Économie/
      result.sub!( /Économie/, 'Sauvegarder' )
    end
    if result =~ / :$/
      #
      # Automatic translation returns "des dossiers" which conflicts with TCLAP
      # command-line flags.  We shorten it to just dossiers.
      #
      result.sub!(/\s:$/, ": ")
    end
  elsif @lang == 'it'
    if result =~ /TEMPO LIBERO/
      result.sub!(/TEMPO LIBERO/, 'OCIO')
    elsif text == '%d Hz.'
      result = text
    end
    result.gsub!( /^(\d*)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
    if text == 'Reel %d (%s) | Shot %d (%s) | Frame %<PRId64> | X = %d | Y = %d\n'
      result = 'Bobina %d (%s) | Colpo %d (%s) | Foto %<PRId64> | X = %d | S = %d\n'
    end
  elsif @lang == 'ja'
    if result =~ /：/
      result.gsub!(/：/, ':')
    end
    if text == "L (Lightness)"
      result = "L （明度）"
    elsif text == "Y' (Lumma)"
      result = "Y' （ルマ）"
    elsif text == "Y (Luminance)"
      result = "Y （輝度）"
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
  elsif @lang == 'ko'
    if text == "Y (Luminance)"
      result = "Y (휘도)"
    elsif text == "L (Lightness)"
      result = "L (가벼움)"
    end
    if result == 'LM변환% 유'
      result = 'LM변환 %u'
    elsif result =~ /여가/
      result.sub!(/여가/, 'OCIO')
    end
  elsif @lang == 'pl'
    if result =~ /Oszczędność/
      result.sub!( /Oszczędność/, 'Nagrywać' )
    elsif result =~ /zapisywania/
      result.sub!( /zapisywania/, 'nagranie' )
    elsif result =~ /Zapisywanie/
      result.sub!( /Zapisywanie/, 'Nagranie' )
    end
  elsif @lang == 'pt'
    if text == '15' or text == '50'
      result = text
    elsif text =~ /set software audio parameters: \%s/
      result = 'Não consegui definir os parâmetros de hardware de áudio: %s'
    elsif text == "xyY CIE xyY"
      result = text
    end
    if result =~ /\\\s+n/
      #
      # Automatic translation returns \ n instead of \n
      #
      result.gsub!(/\\\s+n/, '\n')
    elsif result =~ /LAZER/
      result.gsub!( /LAZER/, "OCIO" )
    elsif result =~ /salvamento/
      result.gsub!( /salvamento/, 'gravação' )
    elsif result =~ /salvar/
      result.gsub!( /salvar/, 'gravar' )
    elsif result =~ /Salvar/
      result.gsub!( /Salvar/, 'Gravar' )
    elsif result =~ /Economia/ or result =~ /Salvando/
      result.gsub!( /Economia/, 'Gravando' )
      result.gsub!( /Salvando/, 'Gravando' )
    end
  elsif @lang == 'ro'
    if result =~ /[^\\]"/
      result.gsub!( /"/, '\"' )
    elsif result =~ /salvarea/
      result.gsub!( /salvarea/, 'înregistrarea' )
    elsif result =~ /Salvare/
      result.gsub!( /Salvare/, 'înregistrare' )
    elsif result =~ /salvați/ or result =~ /Salvați/ or result =~ /Economisire/
      result.gsub!( /Economisire/, 'înregistrați' )
      result.gsub!( /Salvați/, 'înregistrați' )
      result.gsub!( /salvați/, 'înregistrați' )
    end
  elsif @lang == 'ru'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
    if text == '%d Hz.'
      result = text
    elsif text == 'W: %g %g'
      result = 'B: %g %g'
    elsif result =~ /ДОСУГ/
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
  elsif @lang == 'tr'
    result.gsub!( /^(\d+)\.(\d+)(\s+\w)/, '\\1,\\2\\3' )
  elsif @lang == 'zh'
    if text == "Y' (Lumma)"
      result = text
    elsif text == "Y (Luminance)"
      result = text
    elsif text == "L (Lightness)"
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

  if text == 'Arabic'    and @lang != 'ar' and result !~ / \(.*\)/
    result += " (عرب)"
  elsif text == 'English'    and @lang != 'en' and result !~ / \(.*\)/
    result += " (English)"
  elsif text == "Spanish" and @lang != 'es' and result !~ / \(.*\)/
    result += " (Español)"
  elsif text == "German"  and @lang != 'de' and result !~ / \(.*\)/
    result += " (Deutsch)"
  elsif text == "Czech"   and @lang != 'cs' and result !~ / \(.*\)/
    result += " (čeština)"
  elsif text == "French"  and @lang != 'fr' and result !~ / \(.*\)/
    result += " (français)"
  elsif text == "Greek" and @lang != 'gr' and result !~ / \(.*\)/
    result += " (Ελληνικά)"
  elsif text == "Italian" and @lang != 'it' and result !~ / \(.*\)/
    result += " (italiano)"
  elsif text == "Japanese" and @lang != 'ja' and result !~ / \(.*\)/
    result += " (日本)"
  elsif text == "Korean"  and @lang != 'ko' and result !~ / \(.*\)/
    result += " (한국어)"
  elsif text == "Dutch"   and @lang != 'nl' and result !~ / \(.*\)/
    result += " (Nederlands)"
  elsif text == "Polish"  and @lang != 'pl' and result !~ / \(.*\)/
    result += " (Polski)"
  elsif text == "Portuguese" and @lang != 'pt' and result !~ / \(.*\)/
    result += " (português)"
  elsif text == "Romanian"   and @lang != 'ro' and result !~ / \(.*\)/
    result += " (Română)"
  elsif text == "Russian"    and @lang != 'ru' and result !~ / \(.*\)/
    result += " (русский)"
  elsif text == "Swedish"    and @lang != 'sv' and result !~ / \(.*\)/
    result += " (svenska)"
  elsif text == "Turkish"    and @lang != 'tr' and result !~ / \(.*\)/
    result += " (Türk)"
  elsif text == "Chinese"    and @lang != 'zh' and result !~ / \(.*\)/
    result += " (中国人)"
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


def translate( text )
  return fix( text, text ) if @lang == 'en'
  googlelang = @lang
  googlelang = 'el' if @lang == 'gr'
  if text =~ /\//
    menus = text.split('/')
    if menus.size > 1
      puts "#@count origin: #{text} menus" if @debug
      r = @translate.translate menus, from: 'en', to: googlelang
      result = []
      r.each { |m| result << m.text }
      if menus[-1] == '%s'
        result[-1] = '%s'
      end
      result = result.join('/')
      result = fix( text, result )
      return result
    end
  end
  puts "original: #{text}"
  r = @translate.translate text, from: 'en', to: googlelang
  result = r.text
  result = fix( text, result )
  return result
end


@op = nil


def new_line( text )
  puts "#@lang #@count result: #{text}"
  @op.puts "msgstr \"#{text}\""
end

if ARGV.size > 0
  langs = ARGV
else
  langs = [ 'ar', 'cs', 'de', 'en', 'es', 'fr', 'gr', 'it', 'ja',
            'ko', 'nl', 'pl', 'pt', 'ro', 'ru', 'sv', 'tr', 'zh' ]
end

translated = [ ]
for @lang in langs
  next if translated.any? @lang
  $stderr.puts "=================== Translate to #@lang ======================"
  in_msg_id = false
  msg = ''
  root = "#{home}/gga/code/applications/mrv/mrViewer/src/po"
  if not File.exists? "#{root}/#@lang.po"
    $stderr.puts "#{root}/#@lang.po does not exist.  Please run spanish_translate.rb first."
    exit(1)
  end
  fp = File.open("#{root}/#@lang.po.orig", "r", encoding: "utf-8")
  @op = File.open("#{root}/#@lang.po", "w", encoding: "utf-8")

  lines = fp.readlines
  fp.close

  @count = 0 # header

  while @count < 14
    @op.puts lines[@count]
    @count += 1
  end

  fuzzy = false
  @msgid = ''
  num_lines = lines.size-1
  while @count < num_lines
    line = lines[@count]
    line2 = lines[@count+1]
    if in_msg_id
      msgstr = line =~ /^msgstr\s+"/
      line =~ /"(.*)"$/
      match = $1

      text2 = line2 =~ /^"(.*)"$/
      match2 = $1

      if match
        @msgid << match if not fuzzy
      end


      if msgstr
        if (match == '' or fuzzy) and not text2
          r = translate( @msgid )
          new_line( r )
        else
          @op.puts line
        end
        @msgid = ''
        in_msg_id = fuzzy = false
        @count += 1
        next
      end
      @op.puts line
      @count += 1
      next
    else
      if line =~ /fuzzy/
        fuzzy = true
        line.sub!( /fuzzy/, '' )
        @msgid = ''
      end
    end
    msgid = line =~ /^msgid "(.*)"/
    if msgid
      @msgid = $1
      in_msg_id = true
    end
    @op.puts line
    @count += 1
  end
  @op.puts lines[@count]
  @op.close
end
