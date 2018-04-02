
require 'parapin'

class ParallelPort

  COPYRIGHT = '(c) 2007 Neil Stockbridge'
  LICENSE   = 'LGPL'
  REVISION  = '$Revision: 1.1 $'


  module LogicLevel
    LOW  = 0
    HIGH = 1
  end

  class Pin

    # represents a role for a pin
    class Role
      attr_reader :id, :name

      def initialize id, name
        @id = id
        @name = name
      end

      UNSPECIFIED = new -1, 'UNSPECIFIED'
      SENSOR      = new 0,  'SENSOR'
      DRIVER      = new 1,  'DRIVER'
    end

    # encapsulates a type of pin (bidi, input only or ground)
    class Type
      CAN_SENSE     = 1
      CAN_BE_DRIVEN = 2

      def initialize capabilities
        @capabilities = capabilities
      end

      def can_assume_role pin_role
        (@capabilities & CAPAILITY_REQUIRED_FOR_ROLE[pin_role]) != 0
      end

      private
      CAPAILITY_REQUIRED_FOR_ROLE = { Role::SENSOR => CAN_SENSE,
                                      Role::DRIVER => CAN_BE_DRIVEN }
    end


    attr_reader :number, :current_role

    # determines if this pin can assume the given role or not
    def can_assume_role pin_role
      TYPE_OF_PIN[@number - 1].can_assume_role pin_role
    end

    # configures this pin for the given role
    def assume_role pin_role
      @port.check_is_open
      raise "pin #{@number} cannot assume role #{pin_role.name}" unless can_assume_role pin_role
      Parapin.configure_pin @port_fd, @number, pin_role.id
      @current_role = pin_role
    end

    # determines the logic level present at this pin
    def logic_level
      @port.check_is_open
      return @driving_to_level if Role::DRIVER == @current_role
      return Parapin.logic_level_at_pin(@port_fd, @number) if Role::SENSOR == @current_role
      raise "the role for pin #{@number} has not been specified"
    end

    # drives this pin to the given logic level
    def drive logic_level
      @port.check_is_open
      raise "pin #{@number} is not configured as a driver" unless Role::DRIVER == @current_role
      Parapin.drive_pin @port_fd, @number, logic_level
      @driving_to_level = logic_level
    end

    private

    def initialize port, port_fd, number
      @port = port
      @port_fd = port_fd
      @number = number
      @current_role = Role::UNSPECIFIED
    end
  end

  BI_DIRECTIONAL_PIN = Pin::Type.new(Pin::Type::CAN_SENSE + Pin::Type::CAN_BE_DRIVEN)
  INPUT_ONLY_PIN     = Pin::Type.new Pin::Type::CAN_SENSE
  GANGED_PIN         = BI_DIRECTIONAL_PIN
  GROUND_PIN         = Pin::Type.new 0
  TYPE_OF_PIN = [ BI_DIRECTIONAL_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  GANGED_PIN,
                  INPUT_ONLY_PIN,
                  INPUT_ONLY_PIN,
                  INPUT_ONLY_PIN,
                  INPUT_ONLY_PIN,
                  BI_DIRECTIONAL_PIN,
                  INPUT_ONLY_PIN,
                  BI_DIRECTIONAL_PIN,
                  BI_DIRECTIONAL_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN,
                  GROUND_PIN ]

  @pins

  # opens the parallel port
  def self.open
    fd = Parapin::open
    raise "could not open parallel port: #{fd}" if fd < 0
    self.new fd
  end

  # returns the pin with the given number
  def pin pin_number
    check_is_open
    raise "unrecognised pin: #{pin_number}" if pin_number < 1 || 17 < pin_number
    p = @pins[pin_number - 1]
    if p.nil?
      p = Pin.new self, @fd, pin_number
      @pins[pin_number - 1] = p
    end
    p
  end

  # closes this parallel port
  def close
    check_is_open
    Parapin.close @fd
    @open = false
  end

  def check_is_open
    raise 'parallel port is not open' unless @open
  end


  private

  # use 'open' instead
  def initialize with_fd
    @fd = with_fd
    @pins = []
    @open = true
  end

end
# --------------------------------------------------------------- 80 column rule
