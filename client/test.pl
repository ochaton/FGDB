use 5.010;
use strict;
use warnings;

$|++;

use Test::More;
use FindBin;

use lib $FindBin::Bin . '/./';

use FGDB;
use Data::Dumper;

use constant KEYS_AMOUNT => 3;

sub req ($$) {
	return FGDB::send_message(@_);
}

sub do_inserts {
	my $check = shift;
	for my $i (1..KEYS_AMOUNT) {
		my $response = req(INSERT => { "key$i" => "value$i" });
		$check->($response);
	}
}

sub do_peeks {
	for my $i (1..KEYS_AMOUNT) {
		my $response = req(PEEK => "key$i");
		is($response->{code}, 'ERROR');
		is($response->{error}, 'KEY_NOT_FOUND');
	}
}

sub do_deletes {
	my ($check) = @_;
	for my $i (1..KEYS_AMOUNT) {
		my $response = req(DELETE => "key$i");
		$check->($response, "value$i");
	}
}

sub do_selects {
	my $check = shift;
	for my $i (1..KEYS_AMOUNT) {
		my $response = req(SELECT => "key$i");
		$check->($response, "value$i");
	}
}

sub wow_test {
	is_deeply req (INSERT => { "key1" => "val1" }), {
		code => "OK",
		status => "CODE_OK",
	}, "Insert-key1";

	is_deeply req (SELECT => "key1"), {
		code  => "OK",
		value => "val1",
	}, "Select-key1";

	is_deeply req (INSERT => { "key2" => "val2" }), {
		code => "OK",
		status => "CODE_OK",
	}, "Insert-key2";

	is_deeply req (SELECT => "key2"), {
		code  => "OK",
		value => "val2",
	}, "Select-key2";

	is_deeply req (DELETE => "key1"), {
		code  => "OK",
		value => "val1",
	}, "Delete-key1";

	is_deeply req (SELECT => "key1"), {
		code  => "ERROR",
		error => "KEY_NOT_FOUND",
	}, "Select-key1";

	is_deeply req (DELETE => "key2"), {
		code  => "OK",
		value => "val2",
	}, "Delete-key2";

	is_deeply req (SELECT => "key2"), {
		code  => "ERROR",
		error => "KEY_NOT_FOUND",
	}, "Select-key2";
}

sub main {
	my $answers = [];

	map { $answers->[$_] = { "key$_" => "value$_" } } (1..KEYS_AMOUNT);
	do_peeks();
	diag("PEEKS DONE------------");
	do_inserts(sub {
		is($_[0]->{code}, 'OK');
		is($_[0]->{status}, 'CODE_OK');
	});
	do_inserts(
		sub {
			is($_[0]->{code}, 'ERROR');
			is($_[0]->{error}, 'KEY_EXISTS');
		}
	);
	diag("INSERTS DONE----------");
	do_deletes(
		sub {
			is($_[0]->{code}, 'OK');
			is($_[0]->{value}, $_[1]);
		},
	);
	diag("DELETES DONE----------");
	do_inserts(
		sub {
			is($_[0]->{code}, 'OK');
			is($_[0]->{status}, 'CODE_OK');
		}
	);
	diag("INSERTS DONE----------");
	do_selects(
		sub {
			is($_[0]->{code}, 'OK');
			is($_[0]->{value}, $_[1]);
		},
	);
	diag("SELECTS DONE----------");
	do_deletes(
		sub {
			is($_[0]->{code}, 'OK');
			is($_[0]->{value}, $_[1]);
		},
	);
	diag("DELETES DONE----------");
}

wow_test();
main();

done_testing;
