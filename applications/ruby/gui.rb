# Extends C-defined GUI class
class GUI
  def event_loop
    @c_gui = start
    @running = true

    while @running do
      update_view(@c_gui)
      dispatch_input
    end

    close
  end

  def dispatch_input
    input = fetch_input
    return if input.nil?

    handle_input(input)
  end

  def handle_input
  end
end
