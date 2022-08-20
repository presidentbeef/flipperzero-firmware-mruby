class GPIO
  def initialize
    @states = {}
  end

  def on(pin)
    write(pin, 1)
  end

  def off(pin)
    write(pin, 0)
  end

  def write(pin, on_off)
    unless pin.is_a? Integer and pin >= 2 and pin <= 17
      raise "Invalid pin: #{pin}"
    end

    pin_state = @states[pin]
    if pin_state.nil? or pin_state == :input
      set_output(pin)
      @states[pin] = :output
    end

    write_output(pin, on_off)
  end
end
