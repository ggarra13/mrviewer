module AudiosHelper

  def start_column(record)
    seconds_to_time(record.start)
  end

  def duration_column(record)
    seconds_to_time(record.duration)
  end

end
