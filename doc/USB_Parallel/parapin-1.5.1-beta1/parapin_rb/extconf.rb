require 'mkmf'

$CFLAGS = "#{$CFLAGS} -I.."

dir_config("parapin")

create_makefile("parapin")
