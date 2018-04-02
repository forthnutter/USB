
#include <ruby.h>
#include <parapindriver.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


static VALUE rb_mParapin;


static VALUE parapin_open(VALUE module)
{
  int  port = open( "/dev/parapin", 0 );
  return INT2NUM( port );
}


static VALUE parapin_configure_pin(VALUE module, VALUE rb_port, VALUE rb_pin, VALUE rb_direction)
{
  int  port      = NUM2INT( rb_port );
  int  pin       = NUM2INT( rb_pin );
  int  direction = NUM2INT( rb_direction );
  ioctl( port, direction ? PPDRV_IOC_PINMODE_OUT : PPDRV_IOC_PINMODE_IN, LP_PIN[pin] );
  return Qnil;
}


static VALUE parapin_logic_level_at_pin(VALUE module, VALUE rb_port, VALUE rb_pin)
{
  int  port = NUM2INT( rb_port );
  int  pin  = NUM2INT( rb_pin );
  // the ioctl returns 0 for LOW and 65536 for HIGH
  int  level = ioctl( port, PPDRV_IOC_PINGET, LP_PIN[pin] ) >> 16;
  return INT2NUM( level );
}


static VALUE parapin_drive_pin(VALUE module, VALUE rb_port, VALUE rb_pin, VALUE rb_logic_level)
{
  int  port        = NUM2INT( rb_port );
  int  pin         = NUM2INT( rb_pin );
  int  logic_level = NUM2INT( rb_logic_level );
  ioctl( port, logic_level ? PPDRV_IOC_PINSET : PPDRV_IOC_PINCLEAR, LP_PIN[pin] );
  return Qnil;
}


static VALUE parapin_close(VALUE module, VALUE rb_port)
{
  int  port = NUM2INT( rb_port );
  close( port );
  return Qnil;
}


void Init_parapin()
{
  rb_mParapin = rb_define_module("Parapin");

  rb_define_module_function (rb_mParapin, "open", parapin_open, 0);
  rb_define_module_function (rb_mParapin, "configure_pin", parapin_configure_pin, 3);
  rb_define_module_function (rb_mParapin, "logic_level_at_pin", parapin_logic_level_at_pin, 2);
  rb_define_module_function (rb_mParapin, "drive_pin", parapin_drive_pin, 3);
  rb_define_module_function (rb_mParapin, "close", parapin_close, 1);
}
