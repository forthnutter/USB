#use strict;
#use warnings;

my $parashell = "c:/parashell.exe"; #path to parashell.exe

my $delay = .1; #default delay parameter in seconds

#my @bits = (16,32,64,128); #last four bits of the DATA port. Full steps
#my @bits = (1,2,4,8); #first four bits of the DATA port. Full steps
#my @bits = (3,6,12,9); #first four bits of the DATA port. Half steps
my @bits = (48,96,192,144); #last four bits of the DATA port. Half steps

my $rotation = 25; #number of steps to complete one full rotation

#this loop will rotate the stepper motor one complete turn forwards then one complete turn backwards and repeat forever...
while (1){
	for ( my $i=0; $i<=$rotation; $i++ ){
		foreach my $bit (@bits) {
					`$parashell -w $bit`;
			`c:/Docume~1/bcarroll/Desktop/parashell-2.1/parashell-2.1/parashell/bin/Win32/parashell.exe -w $bit`;
			#print "$bit\n"; #debugging...
			delay();
		}
	}

	for ( my $i=0; $i<=$rotation; $i++ ){
		foreach my $bit (reverse @bits) {
			`$parashell -w $bit`;
			#print "$bit\n"; #debugging...
			delay();
		}
	}
}

#turn off all bits so you don't burn up something
`$parashell -w 0`;

sub delay {
	my $d;
	if (@_) { $d = @_; } else { $d = $delay; }
	select(undef,undef,undef, $d);
}
