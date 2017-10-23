package FGDB;

use 5.010;
use strict;
use warnings;
use Socket;
use Data::MessagePack;
use Data::Dumper;

use constant DEBUG => 0;

our %db = (
	host => '127.0.0.1',
	port => 2016,
);

my %COMMANDS = (
	PEEK   => 0x1,
	SELECT => 0x2,
	INSERT => 0x3,
	UPDATE => 0x4,
	DELETE => 0x5,
);

sub send_message {
	my $cmd = shift;
	my $message = pack "V/A*", Data::MessagePack->new->pack([ $COMMANDS{$cmd}, @_ ]);

	# warn $message;
	# warn join " ", split 2, unpack "H*", $message;

	socket my $sock, PF_INET, SOCK_STREAM, getprotobyname("tcp") // die "$! $@";
	warn "Socket created" if DEBUG > 1;
	connect $sock, sockaddr_in($db{port}, inet_aton $db{host}) // die "$! $@";
	warn "Connected to $db{host}:$db{port}" if DEBUG > 1;
	my $bytes = send ($sock, $message, 0) // die "$! $@";
	warn "Sended $bytes bytes" if DEBUG > 1;

	recv ($sock, my $buffer, 4096, 0) // die "$! $@";
	warn "Got from socket: " .(length $buffer) if DEBUG > 1;

	my @reply_code  = qw (OK ERROR FATAL);
	my @fgdb_code   = qw (CODE_OK KEY_EXISTS KEY_NOT_FOUND);
	my @proto_error = qw (UNKNOWN ERROR_COMMAND);


	my $raw;
	eval { $raw = Data::MessagePack->new->unpack($buffer); 1 } or do {
		warn "MessagePack-unpack failed $@";
		warn unpack "H*", $buffer;
		return;
	};
	unless (ref $raw eq 'ARRAY') {
		warn "Reply from server is not an ARRAY";
		say Dumper($raw);
		exit 1;
	}

	my $reply;
	$reply->{code} = $reply_code[$raw->[0]];
	if ($reply->{code} eq 'ERROR') {
		$reply->{error} = $fgdb_code[$raw->[1]];
	} elsif ($reply->{code} eq 'FATAL') {
		$reply->{fatal} = $proto_error[$raw->[1]];
	} else {
		if (ref $raw->[1]) {
			if (ref $raw->[1] ne 'HASH') {
				warn "Expected HASH got ".ref $raw->[1];
				say Dumper ($raw);
				exit 1;
			} else {
				$reply = {
					%$reply,
					%{$raw->[1]},
				};
			}
		} else {
			if ($cmd eq 'INSERT' or $cmd eq 'UPDATE' or $cmd eq 'PEEK') {
				$reply->{status} = $fgdb_code[$raw->[1]];
			} elsif ($cmd eq 'DELETE' or $cmd eq 'SELECT') {
				$reply->{value} = $raw->[1];
			}
		}
	}

	return $reply;
}

1;
