# Methods added to this helper will be available to all templates in the application.
module ApplicationHelper

  def sequence_select( obj, method, show )
    select( obj, method, Sequence.find_by_show(show).collect { |s| [s.name, s.id] } )
  end

  def show_select( obj, method )
    select( obj, method, Show.find(:all).collect { |s| 
              [ "#{s.title} (#{s.name})", s.id] 
            } )
  end

  def seconds_to_time(x)
    hours = (x / 3600).to_i
    x -= hours * 3600
    minutes = (x / 60).to_i
    x -= minutes * 60
    seconds = x.to_i
    ms = (x - seconds) * 1000

    "%02d:%02d:%02d  %03dms" % [hours, minutes, seconds, ms]
  end
end
