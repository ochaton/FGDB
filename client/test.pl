use 5.010;
use strict;
use warnings;

$|++;

use FGDB;
use Data::Dumper;

use constant KEYS_AMOUNT => 3;

sub req {
	my %params = @_;
	return FGDB::send_message(@_);
}

sub do_inserts {
	my $warn_check = shift;
	for my $i (1..KEYS_AMOUNT()) {
		my $response = req(INSERT => { "key$i" => "value$i" });
		warn ("NOT_OK FOR 'INSERT key$i => value$i'\n" . Dumper($response)) unless $warn_check->($response);
	}
}

sub do_peeks {
	my $warn_check = shift;
	for my $i (1..KEYS_AMOUNT()) {
		my $response = req(PEEK => "key$i");
		warn ("NOT_OK FOR 'PEEK key$i'\n" . Dumper($response)) unless $warn_check->($response);
	}
}

sub do_deletes {
	my $warn_check = shift;
	for my $i (1..KEYS_AMOUNT()) {
		my $response = req(DELETE => "key$i");
		warn ("NOT_OK FOR 'DELETE key$i'\n" . Dumper($response)) unless $warn_check->($response);
	}
}

sub do_selects {
	my $warn_check = shift;
	my $correct_answers = shift;
	for my $i (1..KEYS_AMOUNT()) {
		my $response = req(SELECT => "key$i");
		warn ("NOT_OK FOR 'SELECT key$i'\n" . Dumper($response)) unless $warn_check->($response);
		warn "INCORRECT ANSWER FOR 'SELECT key$i'\n" unless $response->{value} eq $correct_answers->[1]->{"key$i"};
	}
}

sub main {
	my $answers = [];
	map { $answers->[$_] = { "key$_" => "value$_" } } (1..KEYS_AMOUNT());
	do_peeks(
		sub {
			return ($_[0]->{error} // '') eq 'KEY_NOT_FOUND';
		}
	);
	print("PEEKS DONE------------\n");
	do_inserts(
		sub {
			return ($_[0]->{code} // '') eq 'OK';
		}
	);
	print("INSERTS DONE----------\n");
	do_selects(
		sub {
			return ($_[0]->{code} // '') eq 'OK';
		},
		$answers
	);
	print("SELECTS DONE----------\n");
	do_inserts(
		sub {
			return ($_[0]->{error} // '') eq 'KEY_EXISTS';
		}
	);
	print("INSERTS DONE----------\n");
	do_deletes(
		sub {
			return ($_[0]->{code} // '') eq 'OK';
		}
	);
	print("DELETES DONE----------\n");
	print("DONE TESTING\n");
}

main();
