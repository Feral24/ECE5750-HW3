#!/bin/sh -- # -*- perl -*- -w
eval 'exec ${PERL:-perl} -w -S $0 ${1+"$@"}'
  if 0;

# prints make header in a nice format for Mark. :)

# First check for any directory aliases in the environment, if necessary
$string = "@ARGV";

if ($string =~ m!/!){
# presumably this is a pathname
   while (($var, $value) = each(%ENV)) {
      if ($value =~ m!/!) {
	 if (($var ne "PWD") && ($var ne "CONFIGDIR") &&
             ($var !~ /PATH/) && ($var ne "TERMCAP")) {
	    $alias{$value} = $var;
	 }
      }
   }
# Try and find pathnames in %ENV and reverse-map them to
# AF stuff. Reverse sort them so that $HOME/blah shows up
# before $HOME
   foreach $dir (reverse(sort(keys(%alias)))) {
      foreach $item (@ARGV) {
	 if ($item =~ /\Q$dir\E/) {
	    $item =~ s/\Q$dir\E/\$($alias{$dir})/;
	 }
      }
   }
}

$line_length = 79;
$tab = 15;			# Determined by Makefile
@ARGV = sort @ARGV;
$i = 0;
grep($lengths[$i++] = length($_) + 1, @ARGV); # for perl4
# @lengths = map { length($_) + 1 } @ARGV; # for perl5

$len = $tab;
for ($i = 0; $i < @ARGV; $i++) {
   $len += $lengths[$i];
   if ($len > $line_length) {
      print "\n", " " x $tab;
      $len = $tab + $lengths[$i];
   }
   print "$ARGV[$i] ";
}
print "\n";

