# Extends C-defined GUI class
class GUI
  def event_loop
    @c_gui = start
    @running = true

    while @running do
      update_view(@c_gui)
      dispatch_input
    end

    close(@c_gui)
  end

  def close_gui
    @running = false
  end

  def display_text(text)
    set_text(text, @c_gui)
  end

  def dispatch_input
    input = fetch_input(@c_gui)
    return if input.nil?

    handle_input(input.to_sym)
  end

  def handle_input
  end
end
