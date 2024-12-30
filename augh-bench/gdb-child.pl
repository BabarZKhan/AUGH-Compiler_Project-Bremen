#! /usr/bin/perl


my $params = "-ex 'up 2' -ex 'set var i=0' -ex 'c'";
#my $params = "-ex 'b cgseddp_explore.c:2729' -ex 'up 2' -ex 'set var i=0' -ex 'c'";

my $a = `ps -Al | grep ughcgs`;
my @al = split(/\s*\n+\s*/, $a);

#print $a;

die("Error, I want 2 processes only !\n") if(@al != 2);

my @line0 = split(/\s+/, $al[0]);
my @line1 = split(/\s+/, $al[1]);

my $pid = -1;
if($line0[4]==$line1[3]) {
	$pid = $line0[3];
}
elsif($line1[4]==$line0[3]) {
	$pid = $line1[3];
}
else {
	die("Error : could not find the child pid !\n");
}

print "PID found : $pid\n";

system("gdb -ex 'attach $pid' $params");

