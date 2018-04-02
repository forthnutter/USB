#parashell  -- Brett Carroll @ 2008
#Read and Write data to/from the parallel port as recieved from the cmdline.
#Usage: parashell.exe [OPTIONS]
#Simple parallel port input/output control program for Windows

#use strict;
#use warnings;
use Device::ParallelPort;
use Getopt::Long;

my $parport = Device::ParallelPort->new('win32'); #load the win32 parallel port driver

my ($mode, $port, $data, $help);

#print usage statement if no command line parameters are passed or there is an unknown parameter or help option is passed
GetOptions('mode=s' => \$mode, 'port=s' => \$port, 'data:i' => \$data, 'help|?' => \$help);

usage if defined $help;

if ($mode =~ /r/i){
	print pin($port); #read from the specified port
} elsif ($mode =~ /w/i){
	pout ($port, $data); #write to the specified port
}

sub pin {
	my $port = shift;
	my $val;
	if ($port =~ /c/i){
		$val = $parport->get_byte(1); #read from the CONTROL port
	} elsif ($port =~ /d/i){
		$val = $parport->get_byte(0); #read from the DATA port
	} elsif ($port =~ /s/i){
		$val = $parport->get_byte(2); #read from the STATUS port
	} else {
		usage();
	}
	return ord($val);
}

sub pout {
	my ($port, $data) = @_;
	my $val;
	for my $i (0 .. 255){
		if ($i == $data){
			if ($port =~ /c/i){
				$parport->set_byte(1,chr($data)); #write to the CONTROL port
			} elsif ($port =~ /d/i){
				$parport->set_byte(0,chr($data)); #write to the DATA port
			} elsif ($port =~ /s/i){
				$parport->set_byte(2,chr($data)); #write to the STATUS port
			}
			return 0;
		}
	}
}

sub usage {
	#Usage statement
	print "Usage: parashell.exe [OPTIONS]\n";
	print "Options:\n";
	print "-mode	Set parashell mode (read or write)\n";
	print "\tr	Set read mode\n";
	print "\tw	Set write mode\n";
	print "-port	Set port to use\n";
	print "\tc	Use CONTROL port\n";
	print "\td	Use DATA port\n";
	print "\ts	Use STATUS port\n";
	print "-data	Specifies data to be sent to the specified port (-mode must be set to w)\n";
	print "       data can be decimal (0-255)\n";
	print "\nExample:\n";
	print "\tc:\\>parashell.exe -mode w -port d -data 255\n";
	print "\tThis example sets all 8 DATA bits high\n";
}