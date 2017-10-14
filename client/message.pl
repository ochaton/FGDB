
use 5.010;
use strict;
use warnings;
use Socket;
use Data::MessagePack;

my %db = (
	host => '127.0.0.1',
	port => 2016,
);

my $message = pack "V/A*", Data::MessagePack->new->pack([0x0, "Message"]);

socket my $sock, PF_INET, SOCK_STREAM, getprotobyname("tcp") // die "$! $@";
say "Socket created";
connect $sock, sockaddr_in($db{port}, inet_aton $db{host}) // die "$! $@";
say "Connected to $db{host}:$db{port}";
my $bytes = send ($sock, $message, 0) // die "$! $@";
say "Sended $bytes bytes";

recv ($sock, my $buffer, 4096, 0) // die "$! $@";
say "Got from socket: $buffer";
