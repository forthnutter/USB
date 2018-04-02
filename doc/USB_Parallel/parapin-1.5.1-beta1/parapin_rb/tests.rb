#!/usr/bin/env ruby
#
# $Revision: 1.1 $
#

require 'parallel-port'


include ParallelPort::LogicLevel


def open_and_close
  port = ParallelPort.open
  puts "opened port #{port}"
  port.close
  port
end


def closed_port
  # check that the class may not be instantiated normally
  check_fails do ParallelPort.new 3 end

  begin
    # open the port
    port = ParallelPort.open
    # get hold of a Pin object to perform closed test later on
    pin = port.pin 1
  ensure
    # close the port
    port.close
  end

  # check that methods fails on the port now it's closed
  check_fails do port.pin 0 end
  check_fails do port.close end
  check_fails do pin.assume_role DRIVER end
  check_fails do pin.logic_level end
  check_fails do pin.drive HIGH end
end


def pin_conf
  begin
    port = ParallelPort.open

    # check that pin#1 can be configured in either direction
    port.pin(1).assume_role DRIVER
    port.pin(1).assume_role SENSOR
  ensure
    port.close
  end
end


def probe
  begin
    port = ParallelPort.open
    probe = port.pin 1
    probe.assume_role ParallelPort::Pin::Role::SENSOR
    while true do
      puts probe.logic_level
      sleep 1
    end
  ensure
    puts 'closing parallel port'
    port.close
  end
end


def flash
  begin
    port = ParallelPort.open
    driver = port.pin 2
    driver.assume_role ParallelPort::Pin::Role::DRIVER
    while true do
      driver.drive LOW == driver.logic_level ? HIGH : LOW
      sleep 1
    end
  ensure
    puts 'closing parallel port'
    port.close
  end
end


def check_fails &test
  begin
    test.call
    raise 'should have barfed'
  rescue
  end
end


def main
  test = ARGV[0]
  self.send test
end

main
