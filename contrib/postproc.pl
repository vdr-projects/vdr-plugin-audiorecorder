#!/usr/bin/perl
use strict;

use File::Basename;

my $LOGGER = "/usr/bin/logger -t " . basename(__FILE__);
my $mydir = dirname(__FILE__);

# write into logfile for debugging purpose
my $logfile;
$logfile= $mydir . '/' . basename(__FILE__, '.pl') . '.log';

# function to write into syslog
sub Log
{
	system("$LOGGER '[audiorecorder]: @_'");
	print LOGFILE "# @_\n" if (defined($logfile));
}

if (defined($logfile)) {
	# open file for debug
	open(LOGFILE, ">> " . $logfile)
	  or die "Fehler beim Öffnen von '$logfile': $!\n";
	print LOGFILE "=========================== ". localtime() . "\n";
	print LOGFILE 'ARGV[0] = ' . $ARGV[0] . "\n";
	print LOGFILE 'ARGV[1] = ' . $ARGV[1] . "\n";
	print LOGFILE 'ARGV[2] = ' . $ARGV[2] . "\n";
	print LOGFILE 'ARGV[3] = ' . $ARGV[3] . "\n";
	print LOGFILE 'ARGV[4] = ' . $ARGV[4] . "\n";
	print LOGFILE 'ARGV[5] = ' . $ARGV[5] . "\n";
	print LOGFILE 'ARGV[6] = ' . $ARGV[6] . "\n";
	print LOGFILE 'ARGV[7] = ' . $ARGV[7] . "\n";
	print LOGFILE 'ARGV[8] = ' . $ARGV[8] . "\n";
	print LOGFILE 'ARGV[9] = ' . $ARGV[9] . "\n";
}

# test number of parameters
if ($#ARGV ne 7) {
	Log("Wrong number of parameters");
	exit;
}

# use telling names for parameters
my $src = $ARGV[0];
my $src_bitrate = $ARGV[1];
my $artist = $ARGV[2];
my $title = $ARGV[3];
my $channel = $ARGV[4];
my $event = $ARGV[5];
my $codec = $ARGV[6];
my $dst_bitrate = $ARGV[7];

# test parameters
if ($artist eq "" || $title eq "") {
	Log("Without data better remove this file");
	exit;
}

# Create canonical filename
my $dst;
my $tmp = lc($artist . "_-_" . $title);
# clear nearly all non-characters
$tmp =~ s/[ \*\?!:"\']/_/g;
# converts slashes to commas
$tmp =~ s/\//,/g;
# convert special characters to ASCII
$tmp =~ s/[áàå]/a/g;
$tmp =~ s/ä/ae/g;
$tmp =~ s/ç/c/g;
$tmp =~ s/[éÉè]/e/g;
$tmp =~ s/[íï]/i/g;
$tmp =~ s/ñ/n/g;
$tmp =~ s/[óò]/o/g;
$tmp =~ s/ö/oe/g;
$tmp =~ s/ù/u/g;
$tmp =~ s/ü/ue/g;
# remove multiple underscores
$tmp =~ s/___*/_/g;
# remove underscore at file end
$tmp =~ s/_$//g;

# Name with extension
$dst = $tmp . "." . $codec;

# open blacklist
my $inname = $mydir . '/blacklist.m3u';
if (open(BLACKLIST, "< " . $inname)) {
	while (<BLACKLIST>) {
		chomp;
		if ($_ eq $dst) {
			Log("File $dst blacklisted");
			exit;
		}
	}
	close(BLACKLIST)
}

# calculate sizes
my $src_size = (-s $src);
print LOGFILE "size($src) = " . $src_size . "\n" if (defined($logfile));

# No file smaller 8 secs
if ($src_size < $src_bitrate) {
    Log("File $src too small ($src_size)!");
    exit;
}

# Full qualified name
my $audiodir = dirname($src) . '/';
my $fqn = $audiodir . $dst;

# file already exists?
if (-e $fqn) {
	# Write duplicate file under curtain circumstances 
	my $size = (-s $fqn); 
	print LOGFILE "size($dst) = " . $size . "\n" if (defined($logfile));
	# If file size is smaller than 17000 bytes this files are special
	if ($size < 17000) {
		Log("File already on CD");
		exit;
	}

	my $dst_size = 0;
	$dst_size = $src_size * $dst_bitrate / $src_bitrate if (!($src_bitrate == 0));
	print LOGFILE "approx. size($dst) = " . $dst_size . "\n" if (defined($logfile));

	if ($size > $dst_size) {
		Log("Already larger version on disk ($size vs. $dst_size)");
		exit;
	} 

		# Files larger than existing ones are stored with
		# a special numbered extension
		my $i = 0;
		while (-e $fqn) {
			$i++;
			$dst = $tmp . ".~" . $i . "~." . $codec;
			$fqn = $audiodir . $dst;
		}
	}

print LOGFILE '$dst = ' . $dst . "\n" if (defined($logfile));
# return file name
print $dst;
