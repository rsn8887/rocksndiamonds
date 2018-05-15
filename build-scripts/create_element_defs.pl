#!/usr/bin/perl -w

# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2014 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# create_element_defs.pl
# =============================================================================

use strict;


# =============================================================================
# C O N F I G U R A T I O N   S E C T I O N
# =============================================================================

my $base_path = ".";
my $src_path = "$base_path/src";

if (-d "../src")	# we're already inside "src" directory
{
    $src_path = ".";
}


# -----------------------------------------------------------------------------
# global variables
# -----------------------------------------------------------------------------

my $filename_header_tmpl = "$src_path/header.tmpl";

my $filename_conf_gfx_h = 'conf_gfx.h';
my $filename_conf_snd_h = 'conf_snd.h';
my $filename_conf_mus_h = 'conf_mus.h';
my $filename_conf_chr_c = 'conf_chr.c';
my $filename_conf_chr_h = 'conf_chr.h';
my $filename_conf_cus_c = 'conf_cus.c';
my $filename_conf_cus_h = 'conf_cus.h';
my $filename_conf_grp_c = 'conf_grp.c';
my $filename_conf_grp_h = 'conf_grp.h';
my $filename_conf_e2g_c = 'conf_e2g.c';
my $filename_conf_esg_c = 'conf_esg.c';
my $filename_conf_e2s_c = 'conf_e2s.c';
my $filename_conf_fnt_c = 'conf_fnt.c';
my $filename_conf_g2s_c = 'conf_g2s.c';
my $filename_conf_g2m_c = 'conf_g2m.c';
my $filename_conf_var_c = 'conf_var.c';
my $filename_conf_act_c = 'conf_act.c';

my $text_auto = 'this file was automatically generated -- do not edit by hand';
my $text_gfx_h = 'values for graphics configuration (normal elements)';
my $text_snd_h = 'values for sounds configuration';
my $text_mus_h = 'values for music configuration';
my $text_chr_c = 'values for graphics configuration (character elements)';
my $text_chr_h = 'values for elements configuration (character elements)';
my $text_cus_c = 'values for graphics configuration (custom elements)';
my $text_cus_h = 'values for elements configuration (custom elements)';
my $text_grp_c = 'values for graphics configuration (group elements)';
my $text_grp_h = 'values for elements configuration (group elements)';
my $text_e2g_c = 'values for element/graphics mapping configuration (normal)';
my $text_esg_c = 'values for element/graphics mapping configuration (special)';
my $text_e2s_c = 'values for element/sounds mapping configuration';
my $text_fnt_c = 'values for font/graphics mapping configuration';
my $text_g2s_c = 'values for gamemode/sound mapping configuration';
my $text_g2m_c = 'values for gamemode/music mapping configuration';
my $text_var_c = 'values for image and layout parameter configuration';
my $text_act_c = 'values for active states of elements and fonts';

my $num_custom_elements = 256;
my $num_group_elements = 32;

my $char_skip = '---[SKIP]---';

my @chars =
    (
     'SPACE',
     'EXCLAM',
     'QUOTEDBL',
     'NUMBERSIGN',
     'DOLLAR',
     'PERCENT',
     'AMPERSAND',
     'APOSTROPHE',
     'PARENLEFT',
     'PARENRIGHT',
     'ASTERISK',
     'PLUS',
     'COMMA',
     'MINUS',
     'PERIOD',
     'SLASH',

     '0',
     '1',
     '2',
     '3',
     '4',
     '5',
     '6',
     '7',
     '8',
     '9',
     'COLON',
     'SEMICOLON',
     'LESS',
     'EQUAL',
     'GREATER',
     'QUESTION',

     'AT',
     'A',
     'B',
     'C',
     'D',
     'E',
     'F',
     'G',
     'H',
     'I',
     'J',
     'K',
     'L',
     'M',
     'N',
     'O',

     'P',
     'Q',
     'R',
     'S',
     'T',
     'U',
     'V',
     'W',
     'X',
     'Y',
     'Z',
     'BRACKETLEFT',
     'BACKSLASH',
     'BRACKETRIGHT',
     'ASCIICIRCUM',
     'UNDERSCORE',

     'COPYRIGHT',
     'AUMLAUT',
     'OUMLAUT',
     'UUMLAUT',
     'DEGREE',
     'TRADEMARK',
     'CURSOR',
     $char_skip,
     $char_skip,
     $char_skip,
     $char_skip,
     $char_skip,
     $char_skip,
     'BUTTON',
     'UP',
     'DOWN',
     );


# -----------------------------------------------------------------------------
# start main program
# -----------------------------------------------------------------------------

main();
exit 0;


# =============================================================================
# F U N C T I O N S
# =============================================================================

sub error
{
    my ($error_msg) = @_;

    print STDERR "ERROR: ";
    print STDERR "$error_msg\n";
}

sub fail
{
    my ($error_msg) = @_;

    print STDERR "FATAL ";
    error("$error_msg");

    exit 1;
}

sub contains_image_file
{
    my ($line) = @_;

    return ($line =~ /\".+\.png\"/ ||
	    $line =~ /UNDEFINED_FILENAME/);
}

sub contains_sound_file
{
    my ($line) = @_;

    return ($line =~ /\".+\.wav\"/ ||
	    $line =~ /UNDEFINED_FILENAME/);
}

sub contains_music_file
{
    my ($line) = @_;

    return ($line =~ /\".+\.wav\"/ ||
	    $line =~ /\".+\.mod\"/ ||
	    $line =~ /\".+\.mp3\"/ ||
	    $line =~ /UNDEFINED_FILENAME/);
}

sub print_file_header
{
    my ($filename, $comment) = @_;
    my $filename_tmpl = 'xxxxxxxx.x';
    my $filename_text = $filename;
    my $filename_def = uc($filename);
    $filename_def =~ s/\./_/;

    $filename_text .= ' ' x (length($filename_tmpl) - length($filename_text));

    open(FILE, "$filename_header_tmpl") ||
	fail("cannot open file '$filename_header_tmpl' for reading");

    while (<FILE>)
    {
	s/$filename_tmpl/$filename_text/;

	print;
    }

    close FILE;

    print "\n";
    print "/* ----- $text_auto ----- */\n";
    print "\n";
    print "#ifndef $filename_def\n";
    print "#define $filename_def\n";
    print "\n";
    print "/* $comment */\n";
    print "\n";
}

sub print_file_footer
{
    my ($filename) = @_;
    my $filename_def = uc($filename);
    $filename_def =~ s/\./_/;

    print "\n";
    print "#endif	/* $filename_def */\n";
}

sub get_tabs
{
    my ($text, $max_num_tabs) = @_;

    my $num_tabs = $max_num_tabs - int(length($text) / 8);

    if ($num_tabs < 1)	# at least one tab needed as separator
    {
	$num_tabs = 1;
    }

    return "\t" x $num_tabs;
}

sub print_graphics_list
{
    my $filename = "$src_path/conf_gfx.c";

    print_file_header($filename_conf_gfx_h, $text_gfx_h);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $max_num_tabs = 7;
    my $i = 0;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/^\#include "conf_chr.c"/)	# dump list of character elements
	{
	    foreach my $char (@chars)
	    {
		my $prefix = $char;

		$prefix =~ s/^/#define IMG_CHAR_/;

		my $tabs = get_tabs($prefix, $max_num_tabs);

		if ($char ne $char_skip)
		{
		    print "$prefix$tabs$i\n";

		    $i++;
		}

		if (lc($char) eq 'space')
		{
		    $prefix =~ s/$/_EDITOR/;

		    my $tabs = get_tabs($prefix, $max_num_tabs);

		    print "$prefix$tabs$i\n";

		    $i++;
		}
	    }

	    foreach my $char (@chars)
	    {
		my $prefix = $char;

		$prefix =~ s/^/#define IMG_STEEL_CHAR_/;

		my $tabs = get_tabs($prefix, $max_num_tabs);

		if ($char ne $char_skip)
		{
		    print "$prefix$tabs$i\n";

		    $i++;
		}

		if (lc($char) eq 'space')
		{
		    $prefix =~ s/$/_EDITOR/;

		    my $tabs = get_tabs($prefix, $max_num_tabs);

		    print "$prefix$tabs$i\n";

		    $i++;
		}
	    }
	}

	if (/^\#include "conf_cus.c"/)	# dump list of custom elements
	{
	    for (my $nr = 0; $nr < $num_custom_elements; $nr++)
	    {
		my $line = sprintf("#define IMG_CUSTOM_%d", $nr + 1);

		my $tabs = get_tabs($line, $max_num_tabs);

		print "$line$tabs$i\n";

		$i++;

		$line = sprintf("#define IMG_CUSTOM_%d_EDITOR", $nr + 1);

		$tabs = get_tabs($line, $max_num_tabs);

		print "$line$tabs$i\n";

		$i++;
	    }
	}

	if (/^\#include "conf_grp.c"/)	# dump list of group elements
	{
	    for (my $nr = 0; $nr < $num_group_elements; $nr++)
	    {
		my $line = sprintf("#define IMG_GROUP_%d", $nr + 1);

		my $tabs = get_tabs($line, $max_num_tabs);

		print "$line$tabs$i\n";

		$i++;

		$line = sprintf("#define IMG_GROUP_%d_EDITOR", $nr + 1);

		$tabs = get_tabs($line, $max_num_tabs);

		print "$line$tabs$i\n";

		$i++;
	    }
	}

	if (!contains_image_file($_))	# skip all lines without image file
	{
	    next;
	}

	s/(.*)/uc($1)/eg;		# convert all characters to upper case
	s/\./_/g;			# replace all '.' with '_'

	s/^  \{ \"/#define IMG_/;	# convert line
	s/\",.*$//;			# convert line

	# dirty hack for making "ABC[DEF]" work as a "special" suffix
	s/([^_])\[/$1_/;

	# dirty hack for making "[default]" work as an element name
	s/\[//;
	s/\]//;

	my $tabs = get_tabs($_, $max_num_tabs);

	print "$_$tabs$i\n";

	$i++;
    }

    my $summary = '#define NUM_IMAGE_FILES';
    my $tabs = get_tabs($summary, $max_num_tabs);

    print "\n$summary$tabs$i\n";

    close FILE;

    print_file_footer($filename_conf_gfx_h);
}

sub print_sounds_list
{
    my %known_element = get_known_element_definitions();

    my $filename = "$src_path/conf_snd.c";

    print_file_header($filename_conf_snd_h, $text_snd_h);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $max_num_tabs = 7;
    my $i = 0;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (!contains_sound_file($_))	# skip all lines without sound file
	{
	    next;
	}

	if (/\[not used\]/ ||
	    / TEST / ||
	    /wav[^\}]*$/)		# skip all lines without sound file
	{
	    next;
	}

	s/(.*)/uc($1)/eg;		# convert all characters to upper case
	s/\./_/g;			# replace all '.' with '_'

	s/^  \{ \"//;
	s/\",.*$//;

	my $sound = $_;

	if ($sound =~ /^\[.+\]/)
	{
	    $sound =~ s/\[//;		# element class sound; begin ...
	    $sound =~ s/\]//;		# ... and end of definition token
	    $sound =~ s/^/CLASS_/;	# add class identifier
 	}

	$sound = "SND_$sound";

	my $define_text = "#define $sound";
	my $tabs = get_tabs($define_text, $max_num_tabs);

	print "$define_text$tabs$i\n";

	$i++;
    }

    my $summary = '#define NUM_SOUND_FILES';
    my $tabs = get_tabs($summary, $max_num_tabs);

    print "\n$summary$tabs$i\n";

    close FILE;

    print_file_footer($filename_conf_snd_h);
}

sub print_music_list
{
    my %known_prefix = get_known_music_prefix_definitions();

    my $filename = "$src_path/conf_mus.c";

    print_file_header($filename_conf_mus_h, $text_mus_h);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $max_num_tabs = 7;
    my $i = 0;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (!contains_music_file($_))	# skip all lines without music file
	{
	    next;
	}

	if (/\[not used\]/ ||
	    / TEST / ||
	    /wav[^\}]*$/)		# skip all lines without music file
	{
	    next;
	}

	s/(.*)/uc($1)/eg;		# convert all characters to upper case
	s/\./_/g;			# replace all '.' with '_'

	s/^  \{ \"//;
	s/\",.*$//;

	my $music = $_;

	$music = "MUS_$music";

	my $define_text = "#define $music";
	my $tabs = get_tabs($define_text, $max_num_tabs);

	print "$define_text$tabs$i\n";

	$i++;
    }

    my $summary = '#define NUM_MUSIC_FILES';
    my $tabs = get_tabs($summary, $max_num_tabs);

    print "\n$summary$tabs$i\n";

    close FILE;

    print_file_footer($filename_conf_mus_h);
}

sub print_chars_elements_list
{
    print_file_header($filename_conf_chr_h, $text_chr_h);

    my $i = 0;

    foreach my $char (@chars)
    {
	my $left = "#define EL_CHAR_$char";

	my $tabs_left = get_tabs($left, 5);

	my $right = sprintf("(EL_CHAR_ASCII0 + %d)", $i + 32);

	if ($char ne $char_skip)
	{
	    print "$left$tabs_left$right\n";
	}

	$i++;
    }

    $i = 0;

    foreach my $char (@chars)
    {
	my $left = "#define EL_STEEL_CHAR_$char";

	my $tabs_left = get_tabs($left, 5);

	my $right = sprintf("(EL_STEEL_CHAR_ASCII0 + %d)", $i + 32);

	if ($char ne $char_skip)
	{
	    print "$left$tabs_left$right\n";
	}

	$i++;
    }

    print_file_footer($filename_conf_chr_c);
}

sub print_chars_graphics_list_line
{
    my ($token, $x, $y) = @_;

    my @extensions =
	(
	 '',
	 '.xpos',
	 '.ypos',
	 '.frames',
	 );

    my $basename = ($token =~ /^steel_char/ ? 'RocksFontDC' : 'RocksFontEM' );

    foreach my $ext (@extensions)
    {
	my $left = "  \{ \"$token$ext\",";

	my $tabs_left = get_tabs($left, 6);

	my $right = ($ext eq '' ? $basename . '.png' :
		     $ext eq '.frames' ? '1' : '0');

	if ($ext eq '.xpos')
	{
	    $right = $x;
	}
	elsif ($ext eq '.ypos')
	{
	    $right = $y;
	}

	$right = "\"$right\"";

	my $tabs_right = get_tabs($right, 3);

	print "$left$tabs_left$right$tabs_right},\n";
    }
}

sub print_chars_graphics_list
{
    print_file_header($filename_conf_chr_c, $text_chr_c);

    my $i = 0;

    foreach my $char (@chars)
    {
	if ($char ne $char_skip)
	{
	    my $x = $i % 16;
	    my $y = int($i / 16);

	    print_chars_graphics_list_line(lc("char_$char"), $x, $y);
	}

	if (lc($char) eq 'space')
	{
	    print_chars_graphics_list_line("char_space.EDITOR", 7, 4);
	}

	if ($char ne $char_skip)
	{
	    print "\n";
	}

	$i++;
    }

    $i = 0;

    foreach my $char (@chars)
    {
	if ($char ne $char_skip)
	{
	    my $x = $i % 16;
	    my $y = int($i / 16);

	    print_chars_graphics_list_line(lc("steel_char_$char"), $x, $y);
	}

	if (lc($char) eq 'space')
	{
	    print_chars_graphics_list_line("steel_char_space.EDITOR", 7, 4);
	}

	if ($char ne $char_skip)
	{
	    print "\n";
	}

	$i++;
    }

    print_file_footer($filename_conf_chr_c);
}

sub print_custom_elements_list
{
    print_file_header($filename_conf_cus_h, $text_cus_h);

    for (my $i = 0; $i < $num_custom_elements; $i++)
    {
	my $left = sprintf("#define EL_CUSTOM_%d", $i + 1);

	my $tabs_left = get_tabs($left, 5);

	my $right = "(EL_CUSTOM_START + $i)";

	print "$left$tabs_left$right\n";
    }

    print_file_footer($filename_conf_cus_c);
}

sub print_group_elements_list
{
    print_file_header($filename_conf_grp_h, $text_grp_h);

    for (my $i = 0; $i < $num_group_elements; $i++)
    {
	my $left = sprintf("#define EL_GROUP_%d", $i + 1);

	my $tabs_left = get_tabs($left, 5);

	my $right = "(EL_GROUP_START + $i)";

	print "$left$tabs_left$right\n";
    }

    print_file_footer($filename_conf_grp_c);
}

sub print_custom_graphics_list
{
    my @extensions1 =
	(
	 '',
	 '.xpos',
	 '.ypos',
	 '.frames',
	 );
    my @extensions2 =
	(
	 '',
	 '.xpos',
	 '.ypos',
	 );

    print_file_header($filename_conf_cus_c, $text_cus_c);

    for (my $i = 0; $i < $num_custom_elements; $i++)
    {
	foreach my $ext (@extensions1)
	{
	    my $left = sprintf("  \{ \"custom_%d$ext\",", $i + 1);

	    my $tabs_left = get_tabs($left, 6);

	    # my $right = ($ext eq '' ? 'RocksElements.png' :
	    my $right = ($ext eq '' ? 'RocksCE.png' :
			 $ext eq '.frames' ? '1' : '0');

	    if ($ext eq '.xpos')
	    {
		# $right = 7;
		$right = int($i % 16);
	    }
	    elsif ($ext eq '.ypos')
	    {
		# $right = 9;
		$right = int($i / 16);
	    }

	    $right = "\"$right\"";

	    my $tabs_right = get_tabs($right, 3);

	    print "$left$tabs_left$right$tabs_right},\n";
	}

	foreach my $ext (@extensions2)
	{
	    my $left = sprintf("  \{ \"custom_%d.EDITOR$ext\",", $i + 1);

	    my $tabs_left = get_tabs($left, 6);

	    # my $right = ($ext eq '' ? 'RocksElements.png' : '0');
	    my $right = ($ext eq '' ? 'RocksCE.png' : '0');

	    if ($ext eq '.xpos')
	    {
		# $right = 15;
		$right = int($i % 16) + 16;
	    }
	    elsif ($ext eq '.ypos')
	    {
		# $right = 13;
		$right = int($i / 16);
	    }

	    $right = "\"$right\"";

	    my $tabs_right = get_tabs($right, 3);

	    print "$left$tabs_left$right$tabs_right},\n";
	}

	print "\n";
    }

    print_file_footer($filename_conf_cus_c);
}

sub print_group_graphics_list
{
    my @extensions1 =
	(
	 '',
	 '.xpos',
	 '.ypos',
	 '.frames',
	 );
    my @extensions2 =
	(
	 '',
	 '.xpos',
	 '.ypos',
	 );

    print_file_header($filename_conf_grp_c, $text_grp_c);

    for (my $i = 0; $i < $num_group_elements; $i++)
    {
	foreach my $ext (@extensions1)
	{
	    my $left = sprintf("  \{ \"group_%d$ext\",", $i + 1);

	    my $tabs_left = get_tabs($left, 6);

	    # my $right = ($ext eq '' ? 'RocksDC.png' :
	    my $right = ($ext eq '' ? 'RocksCE.png' :
			 $ext eq '.frames' ? '1' : '0');

	    if ($ext eq '.xpos')
	    {
		# $right = 4;
		$right = int($i % 16);
	    }
	    elsif ($ext eq '.ypos')
	    {
		# $right = 15;
		$right = int($i / 16) + int($num_custom_elements / 16);
	    }

	    $right = "\"$right\"";

	    my $tabs_right = get_tabs($right, 3);

	    print "$left$tabs_left$right$tabs_right},\n";
	}

	foreach my $ext (@extensions2)
	{
	    my $left = sprintf("  \{ \"group_%d.EDITOR$ext\",", $i + 1);

	    my $tabs_left = get_tabs($left, 6);

	    # my $right = ($ext eq '' ? 'RocksDC.png' : '0');
	    my $right = ($ext eq '' ? 'RocksCE.png' : '0');

	    if ($ext eq '.xpos')
	    {
		# $right = 14;
		$right = int($i % 16) + 16;
	    }
	    elsif ($ext eq '.ypos')
	    {
		# $right = 15;
		$right = int($i / 16) + int($num_custom_elements / 16);
	    }

	    $right = "\"$right\"";

	    my $tabs_right = get_tabs($right, 3);

	    print "$left$tabs_left$right$tabs_right},\n";
	}

	print "\n";
    }

    print_file_footer($filename_conf_grp_c);
}

sub get_known_element_definitions_ALTERNATIVE
{
    my %known_element = ();

    my $filename = "$src_path/main.h";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	# process line with element definition
	if (/^\#define (EL_[A-Z0-9_]+)\s/)
	{
	    $known_element{$1} = 1;

	    # print STDERR "known_element: '$1'\n";
	}
    }

    close FILE;

    return %known_element;
}

sub get_known_element_definitions
{
    my %known_element = ();

    my $filename = "$src_path/main.c";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $element_name = '';
    my $line_is_element_name = 0;
    my $skip_line = 1;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/ELEMENT_INFO_START/)	# keyword to start parsing file
	{
	    $skip_line = 0;
	    next;
	}
	elsif (/ELEMENT_INFO_END/)	# keyword to stop parsing file
	{
	    last;
	}
	elsif ($skip_line)
	{
	    next;
	}

	if (/^\s+\{\s*$/)
	{
	    $line_is_element_name = 1;
	}
	elsif ($line_is_element_name)
	{
	    # process line with element name definition
	    if (/^\s+\"(.+)\",?\s*$/)
	    {
		$element_name = 'EL_' . uc($1);

		# dirty hack for making "[default]" work as an element name
		$element_name =~ s/\[//;
		$element_name =~ s/\]//;

		# change '.' to '_' for elements like "dynamite.active"
		$element_name =~ s/\./_/g;

		$known_element{$element_name} = 1;

		# printf STDERR "::: known element '$element_name'\n";
	    }

	    $line_is_element_name = 0;
	}
    }

    close FILE;

    return %known_element;
}

sub get_known_element_class_definitions
{
    my %known_element_class = ();

    my $filename = "$src_path/main.c";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $element_name = '';
    my $element_class = '';
    my $line_is_element_name = 0;
    my $line_is_element_class = 0;
    my $skip_line = 1;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/ELEMENT_INFO_START/)	# keyword to start parsing file
	{
	    $skip_line = 0;
	    next;
	}
	elsif (/ELEMENT_INFO_END/)	# keyword to stop parsing file
	{
	    last;
	}
	elsif ($skip_line)
	{
	    next;
	}

	if (/^\s+\{\s*$/)
	{
	    $line_is_element_name = 1;
	}
	elsif ($line_is_element_name)
	{
	    # process line with element name definition
	    if (/^\s+\"(.+)\",?\s*$/)
	    {
		$element_name = 'EL_' . uc($1);

		# dirty hack for making "[default]" work as an element name
		$element_name =~ s/\[//;
		$element_name =~ s/\]//;

		# change '.' to '_' for elements like "dynamite.active"
		$element_name =~ s/\./_/g;
	    }

	    $line_is_element_name = 0;
	    $line_is_element_class = 1;
	}
	elsif ($line_is_element_class)
	{
	    # process line with element class definition
	    if (/^\s+\"(.+)\",?\s*$/)
	    {
		$element_class = 'EL_CLASS_' . uc($1);

		if (!defined($known_element_class{$element_class}))
		{
		    $known_element_class{$element_class} = $element_name;
		}

		$known_element_class{$element_name} = $element_class;

		# print STDERR "known_element_class: '$element_name' => '$element_class'\n";
	    }

	    $line_is_element_class = 0;
	}
    }

    close FILE;

    return %known_element_class;
}

sub get_known_action_definitions
{
    my %known_action = ();

    my $filename = "$src_path/main.h";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	# process line with action definition
	if (/^\#define ACTION_([A-Z0-9_]+)\s/)
	{
	    $known_action{$1} = 1;

	    # print STDERR "known_action: '$1'\n";
	}
    }

    close FILE;

    return %known_action;
}

sub get_known_special_arg_definitions
{
    my %known_special_arg = ();

    my $filename = "$src_path/main.h";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	# process line with special arg definition
	if (/^\#define GFX_SPECIAL_ARG_([A-Z0-9_]+)\s/)
	{
	    if ($1 eq 'CRUMBLED')
	    {
		next;
	    }

	    $known_special_arg{$1} = 1;

	    # print STDERR "known_special_arg: '$1'\n";
	}
    }

    close FILE;

    return %known_special_arg;
}

sub get_known_button_definitions
{
    my %known_button = ();

    my $filename = "$src_path/conf_gfx.h";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	# process line with button definition
	if (/^\#define (IMG_MENU_BUTTON[A-Z0-9_]*)\s/)
	{
	    $known_button{$1} = 1;

	    # print STDERR "known_button: '$1'\n";
	}
    }

    close FILE;

    return %known_button;
}

sub get_known_font_definitions
{
    my %known_font = ();

    my $filename = "$src_path/main.h";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	# process line with font definition
	if (/^\#define (FONT_[A-Z0-9_]+)\s/)
	{
	    $known_font{$1} = 1;

	    # print STDERR "known_font: '$1'\n";
	}
    }

    close FILE;

    return %known_font;
}

sub get_known_sound_prefix_definitions
{
    my %known_sound_prefix = ( 'background'	=> 1 );

    return %known_sound_prefix;
}

sub get_known_music_prefix_definitions
{
    my %known_music_prefix = ();

    my $filename = "$src_path/main.c";

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    my $prefix_name = '';
    my $skip_line = 1;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/MusicPrefixInfo/)		# keyword to start parsing file
	{
	    $skip_line = 0;
	    next;
	}
	elsif (/NULL/ && !$skip_line)	# keyword to stop parsing file
	{
	    last;
	}
	elsif ($skip_line)
	{
	    next;
	}

	if (/^\s+{\s+\"(.+)\"/)
	{
	    my $music_prefix = $1;

	    $known_music_prefix{$music_prefix} = 1;

	    # printf STDERR "::: known music prefix '$music_prefix'\n";
	}
    }

    close FILE;

    return %known_music_prefix;
}

sub print_element_to_graphic_entry
{
    my ($element, $action, $direction, $crumbled, $graphic) = @_;

    my $num_tabs = 5 - int((length($element) + 4 + 1) / 8);
    my $tabs = "\t" x $num_tabs;
    if ($tabs eq '')
    {
	$tabs = ' ';
    }

    $crumbled = ($crumbled == 1 ? 'TRUE' : 'FALSE');

    print "  {\n";
    print "    $element,$tabs$action, $direction, $crumbled,\n";
    print "    $graphic\n";
    print "  },\n";
}

sub print_element_to_special_graphic_entry
{
    my ($element, $special, $graphic) = @_;

    my $num_tabs = 6 - int((length($element) + 4 + 1) / 8);
    my $tabs = "\t" x $num_tabs;
    if ($tabs eq '')
    {
	$tabs = ' ';
    }

    print "  {\n";
    print "    $element,$tabs$special,\n";
    print "    $graphic\n";
    print "  },\n";
}

sub print_font_to_graphic_entry
{
    my ($font, $special, $graphic) = @_;

    my $num_tabs = 6 - int((length($font) + 4 + 1) / 8);
    my $tabs = "\t" x $num_tabs;
    if ($tabs eq '')
    {
	$tabs = ' ';
    }

    print "  {\n";
    print "    $font,$tabs$special,\n";
    print "    $graphic\n";
    print "  },\n";
}

sub print_element_to_sound_entry
{
    my ($element, $is_class, $action, $sound) = @_;

    my $element_plus_is_class = "$element, $is_class";

    my $num_tabs = 6 - int((length($element_plus_is_class) + 4 + 1) / 8);
    my $tabs = "\t" x $num_tabs;
    if ($tabs eq '')
    {
	$tabs = ' ';
    }

    print "  {\n";
    print "    $element_plus_is_class,$tabs$action,\n";
    print "    $sound\n";
    print "  },\n";
}

sub print_gamemode_to_sound_entry
{
    my ($gamemode, $sound) = @_;

    print "  {\n";
    print "    $gamemode,\n";
    print "    $sound\n";
    print "  },\n";
}

sub print_gamemode_to_music_entry
{
    my ($gamemode, $music) = @_;

    print "  {\n";
    print "    $gamemode,\n";
    print "    $music\n";
    print "  },\n";
}

sub print_image_config_var_entry
{
    my ($token, $var) = @_;

    print "  {\n";
    print "    $token,\n";
    print "    $var\n";
    print "  },\n";
}

sub print_active_state_entry
{
    my ($token, $token_active) = @_;

    print "  {\n";
    print "    $token,\n";
    print "    $token_active\n";
    print "  },\n";
}

sub print_element_to_graphic_list
{
    my %graphic_without_element =
	(
	 'IMG_FLAMES_1_LEFT'		=> 1,
	 'IMG_FLAMES_2_LEFT'		=> 1,
	 'IMG_FLAMES_3_LEFT'		=> 1,
	 'IMG_FLAMES_1_RIGHT'		=> 1,
	 'IMG_FLAMES_2_RIGHT'		=> 1,
	 'IMG_FLAMES_3_RIGHT'		=> 1,
	 'IMG_FLAMES_1_UP'		=> 1,
	 'IMG_FLAMES_2_UP'		=> 1,
	 'IMG_FLAMES_3_UP'		=> 1,
	 'IMG_FLAMES_1_DOWN'		=> 1,
	 'IMG_FLAMES_2_DOWN'		=> 1,
	 'IMG_FLAMES_3_DOWN'		=> 1,
	 'IMG_TWINKLE_BLUE'		=> 1,
	 'IMG_TWINKLE_WHITE'		=> 1,
	 );

    my %additional_mappings =
	(
	 # file elements which are mapped to runtime elements when playing

	 # 'EL_EM_KEY_1_FILE'		=> 'IMG_EM_KEY_1',
	 # 'EL_EM_KEY_2_FILE'		=> 'IMG_EM_KEY_2',
	 # 'EL_EM_KEY_3_FILE'		=> 'IMG_EM_KEY_3',
	 # 'EL_EM_KEY_4_FILE'		=> 'IMG_EM_KEY_4',

	 # new elements which still have no graphic
	 # 'EL_DOOR_WHITE',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_DOOR_WHITE_GRAY',	=> 'IMG_CHAR_QUESTION',
	 # 'EL_KEY_WHITE',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_RADIOACTIVITY',	=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_WHEELCHAIR',	=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_PARKING',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_ONEWAY',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_HEART',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_TRIANGLE',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_ROUND',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_EXIT',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_YINYANG',		=> 'IMG_CHAR_QUESTION',
	 # 'EL_SIGN_OTHER',		=> 'IMG_CHAR_QUESTION',
	 'EL_SIGN_UNUSED_1',		=> 'IMG_CHAR_QUESTION',
	 'EL_SIGN_UNUSED_2',		=> 'IMG_CHAR_QUESTION',
	 'EL_DX_UNKNOWN_15',		=> 'IMG_CHAR_QUESTION',
	 'EL_DX_UNKNOWN_42',		=> 'IMG_CHAR_QUESTION',

	 # file elements with direction which is not defined
	 'EL_BD_BUTTERFLY_LEFT'		=> 'IMG_BD_BUTTERFLY',
	 'EL_BD_BUTTERFLY_RIGHT'	=> 'IMG_BD_BUTTERFLY',
	 'EL_BD_BUTTERFLY_UP'		=> 'IMG_BD_BUTTERFLY',
	 'EL_BD_BUTTERFLY_DOWN'		=> 'IMG_BD_BUTTERFLY',
	 'EL_BD_FIREFLY_LEFT'		=> 'IMG_BD_FIREFLY',
	 'EL_BD_FIREFLY_RIGHT'		=> 'IMG_BD_FIREFLY',
	 'EL_BD_FIREFLY_UP'		=> 'IMG_BD_FIREFLY',
	 'EL_BD_FIREFLY_DOWN'		=> 'IMG_BD_FIREFLY',
	 );

    my @unknown_graphics = ();
    my %known_element     = get_known_element_definitions();
    my %known_action      = get_known_action_definitions();
    my %known_special_arg = get_known_special_arg_definitions();
    my %known_direction =
	(
	 'LEFT'		=> 1,
	 'RIGHT'	=> 1,
	 'UP'		=> 1,
	 'DOWN'		=> 1,
	 'UPLEFT'	=> 1,
	 'UPRIGHT'	=> 1,
	 'DOWNLEFT'	=> 1,
	 'DOWNRIGHT'	=> 1,
	 );

    # ---------- read graphic file definitions ----------

    my $filename = "$src_path/conf_gfx.c";

    print_file_header($filename_conf_e2g_c, $text_e2g_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int element;\n";
    print "  int action;\n";
    print "  int direction;\n";
    print "  boolean crumbled;\n";
    print "\n";
    print "  int graphic;\n";
    print "}\n";
    print "element_to_graphic[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/NO_MORE_ELEMENT_IMAGES/)	# keyword to stop parsing file
	{
	    last;
	}

	if (!contains_image_file($_))	# skip all lines without image file
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	s/\[(\d+)\]/_$1/;		# convert "[1]" to "_1" etc.

	s/\[//;				# dirty hack for making "[default]" ...
	s/\]//;				# ... work as an element name

	my $token = $_;

	if ($token =~ /\.([^\.]+)$/ && defined($known_special_arg{$1}))
	{
	    next;			# skip all special definitions
	}

	$token = uc($token);		# convert all characters to upper case

	my $gfx_action_default = '-1';
	my $gfx_action = $gfx_action_default;

	my $gfx_direction_default = '-1';
	my $gfx_direction = $gfx_direction_default;

	my $gfx_crumbled = '0';

	my $object = $token;
	my $action = '';
	my $direction = '';
	my $crumbled = '';

	if ($object =~ /^(.*)\.([A-Z0-9]+)$/ && $2 eq 'CRUMBLED')
	{
	    $object = $1;
	    $crumbled = $2;

	    $gfx_crumbled = '1';
	}

	if ($object =~ /^(.*)\.([A-Z0-9]+)$/ && defined($known_direction{$2}))
	{
	    $object = $1;
	    $direction = $2;

	    $gfx_direction = "MV_BIT_$direction";
	    $gfx_direction_default = $gfx_direction;
	}

	if ($object =~ /^(.*)\.([A-Z0-9_]+)$/ && defined($known_action{$2}))
	{
	    $object = $1;
	    $action = $2;

	    $gfx_action = "ACTION_$action";
	    $gfx_action_default = $gfx_action;
	}

	$token =~ s/\./_/g;
	$object =~ s/\./_/g;	# needed for "invisible_sand.active.digging"

	# print STDERR "'$token' => '$object', '$action', '$direction'\n";

	my $full_element = "EL_$token";
	my $base_element = "EL_$object";

	my $element = $base_element;
	my $graphic = "IMG_$token";

	my $element_without_crumbled = $full_element;
	$element_without_crumbled =~ s/_$crumbled$//;

	my $element_without_direction = $element_without_crumbled;
	$element_without_direction =~ s/_$direction$//;

	my $element_without_action = $element_without_direction;
	$element_without_action =~ s/_$action$//;

	if (defined($known_element{$full_element}))
	{
	    $element = $full_element;

	    $gfx_action_default = '-1';
	    $gfx_direction_default = '-1';
	}

	if ($element_without_action eq $element || $action eq '')
	{
	    $element_without_action = '';
	}

	if ($element_without_direction eq $element || $direction eq '')
	{
	    $element_without_direction = '';
	}

	if ($element_without_crumbled eq $element || $crumbled eq '')
	{
	    $element_without_crumbled = '';
	}

	if (defined($graphic_without_element{$graphic}))
	{
	    next;
	}

	if (!defined($known_element{$element}))
	{
	    # print STDERR "----- ERROR: unknown element '$element' -----\n";

	    push @unknown_graphics, $graphic;

	    next;
	}

	if (defined($known_element{$element}))
	{
	    print_element_to_graphic_entry($element,
					   $gfx_action_default,
					   $gfx_direction_default,
					   $gfx_crumbled,
					   $graphic);
	}

	if (defined($known_element{$element_without_action}))
	{
	    print_element_to_graphic_entry($element_without_action,
					   $gfx_action,
					   $gfx_direction,
					   $gfx_crumbled,
					   $graphic);
	}

	if (defined($known_element{$element_without_direction}))
	{
	    print_element_to_graphic_entry($element_without_direction,
					   '-1',
					   $gfx_direction,
					   $gfx_crumbled,
					   $graphic);
	}

	if (defined($known_element{$element_without_crumbled}))
	{
	    print_element_to_graphic_entry($element_without_crumbled,
					   '-1',
					   '-1',
					   $gfx_crumbled,
					   $graphic);
	}
    }

    # dump list of additional elements
    foreach my $element (sort keys %additional_mappings)
    {
	print_element_to_graphic_entry($element, '-1', '-1', '-1',
				       $additional_mappings{$element});
    }

    # dump list of character elements
    foreach my $char (@chars)
    {
	my $element = "EL_CHAR_$char";
	my $graphic = "IMG_CHAR_$char";

	if ($char ne $char_skip)
	{
	    print_element_to_graphic_entry($element, '-1', '-1', '-1',$graphic);
	}
    }
    foreach my $char (@chars)
    {
	my $element = "EL_STEEL_CHAR_$char";
	my $graphic = "IMG_STEEL_CHAR_$char";

	if ($char ne $char_skip)
	{
	    print_element_to_graphic_entry($element, '-1', '-1', '-1',$graphic);
	}
    }

    # dump list of custom elements
    for (my $i = 0; $i < $num_custom_elements; $i++)
    {
	my $element = sprintf("EL_CUSTOM_%d", $i + 1);
	my $graphic = sprintf("IMG_CUSTOM_%d", $i + 1);

	print_element_to_graphic_entry($element, '-1', '-1', '-1', $graphic);
    }

    # dump list of group elements
    for (my $i = 0; $i < $num_group_elements; $i++)
    {
	my $element = sprintf("EL_GROUP_%d", $i + 1);
	my $graphic = sprintf("IMG_GROUP_%d", $i + 1);

	print_element_to_graphic_entry($element, '-1', '-1', '-1', $graphic);
    }

    print_element_to_graphic_entry('-1', '-1', '-1', '-1', '-1');

    print "};\n";

    close FILE;

    if (scalar(@unknown_graphics) > 0)
    {
	print STDERR "-" x 79 . "\n";
	print STDERR "The following graphics cannot be associated with any element:\n";

	foreach my $graphic (@unknown_graphics)
	{
	    print STDERR "- $graphic\n";
	}

	print STDERR "-" x 79 . "\n";
    }

    print_file_footer($filename_conf_e2g_c);
}

sub print_element_to_special_graphic_list
{
    my %graphic_without_element =
	(
	 'IMG_GLOBAL_DOOR'		=> 1,
	 );

    my %additional_mappings =
	(
	 # old elements which are mapped to other elements when playing
	 #'EL_BUG'			=> 'IMG_BUG_RIGHT',
	 #'EL_SPACESHIP'		=> 'IMG_SPACESHIP_RIGHT',
	 #'EL_PACMAN'			=> 'IMG_PACMAN_RIGHT',
	 );

    my @elements_with_editor_graphic =
	(
	 'char_space'
	 );

    my @unknown_graphics = ();
    my %known_element     = get_known_element_definitions();
    my %known_special_arg = get_known_special_arg_definitions();

    # ---------- read graphic file definitions ----------

    my $filename = "$src_path/conf_gfx.c";

    print_file_header($filename_conf_esg_c, $text_esg_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int element;\n";
    print "  int special;\n";
    print "\n";
    print "  int graphic;\n";
    print "}\n";
    print "element_to_special_graphic[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/NO_MORE_ELEMENT_IMAGES/)	# keyword to stop parsing file
	{
	    last;
	}

	if (!contains_image_file($_))	# skip all lines without image file
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	my $token = $_;
	my $object;
	my $special;

	if ($token =~ /^font\./)	# skip all font definitions
	{
	    next;
	}

	if ($token =~ /^background\./)	# skip all background image definitions
	{
	    next;
	}

	if ($token =~ /^(.*)\.([^\.]+)$/ &&
	    defined($known_special_arg{$2}))
	{
	    $object = $1;
	    $special = "GFX_SPECIAL_ARG_" . $2;
	}
	else
	{
	    next;			# skip all default definitions
	}

	$token  =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case
	$object =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case

	$token  =~ s/\./_/g;
	$object =~ s/\./_/g;

	# print STDERR "'$token' => '$object'\n";

	my $element = "EL_$object";
	my $graphic = "IMG_$token";

	if (defined($graphic_without_element{$graphic}))
	{
	    next;
	}

	if (!defined($known_element{$element}))
	{
	    # print STDERR "----- ERROR: unknown element '$element' -----\n";

	    push @unknown_graphics, $graphic;

	    next;
	}

	print_element_to_special_graphic_entry($element,
					       $special,
					       $graphic);
    }

    # dump list of additional elements
    foreach my $element (sort keys %additional_mappings)
    {
	print_element_to_special_graphic_entry($element,
					       'GFX_SPECIAL_ARG_EDITOR',
					       $additional_mappings{$element});
	print_element_to_special_graphic_entry($element,
					       'GFX_SPECIAL_ARG_PREVIEW',
					       $additional_mappings{$element});
    }

    # dump list of custom element editor graphics
    for (my $i = 0; $i < $num_custom_elements; $i++)
    {
	my $element = sprintf("EL_CUSTOM_%d", $i + 1);
	my $graphic = sprintf("IMG_CUSTOM_%d_EDITOR", $i + 1);

	print_element_to_special_graphic_entry($element,
					       'GFX_SPECIAL_ARG_EDITOR',
					       $graphic);
    }

    # dump list of group element editor graphics
    for (my $i = 0; $i < $num_group_elements; $i++)
    {
	my $element = sprintf("EL_GROUP_%d", $i + 1);
	my $graphic = sprintf("IMG_GROUP_%d_EDITOR", $i + 1);

	print_element_to_special_graphic_entry($element,
					       'GFX_SPECIAL_ARG_EDITOR',
					       $graphic);
    }

    # dump other special editor graphics
    foreach my $token (@elements_with_editor_graphic)
    {
	my $element = 'EL_'  . uc($token);
	my $graphic = 'IMG_' . uc($token) . '_EDITOR';

	print_element_to_special_graphic_entry($element,
					       'GFX_SPECIAL_ARG_EDITOR',
					       $graphic);
    }

    print_element_to_special_graphic_entry('-1', '-1', '-1');

    print "};\n";

    close FILE;

    if (scalar(@unknown_graphics) > 0)
    {
	print STDERR "-" x 79 . "\n";
	print STDERR "The following graphics cannot be associated with any element:\n";

	foreach my $graphic (@unknown_graphics)
	{
	    print STDERR "- $graphic\n";
	}

	print STDERR "-" x 79 . "\n";
    }

    print_file_footer($filename_conf_esg_c);
}

sub print_element_to_sound_list
{
    my %sound_without_action =
	(
	 'SND_AMOEBA_TURNING_TO_GEM'		=> 1,
	 'SND_AMOEBA_TURNING_TO_ROCK'		=> 1,
	 'SND_BD_AMOEBA_TURNING_TO_GEM'		=> 1,
	 'SND_BD_AMOEBA_TURNING_TO_ROCK'	=> 1,

	 # no special case anymore after adding action ".splashing"
	 # 'SND_ACID_SPLASHING'			=> 1,
	 );

    my @unknown_sounds = ();
    my %known_element       = get_known_element_definitions();
    my %known_element_class = get_known_element_class_definitions();
    my %known_action        = get_known_action_definitions();

    # ---------- read sound file definitions ----------

    my $filename = "$src_path/conf_snd.c";

    print_file_header($filename_conf_e2s_c, $text_e2s_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int element;\n";
    print "  boolean is_class;\n";
    print "  int action;\n";
    print "\n";
    print "  int sound;\n";
    print "}\n";
    print "element_to_sound[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/NO_MORE_ELEMENT_SOUNDS/)	# keyword to stop parsing file
	{
	    last;
	}

	if (!contains_sound_file($_))	# skip all lines without sound file
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	my $token = $_;

	$token = uc($token);		# convert all characters to upper case

	my $snd_action_default = '-1';
	my $snd_action = $snd_action_default;

	my $object = $token;
	my $action = '';

	if ($object =~ /^(.*)\.([A-Z0-9_]+)$/ && defined($known_action{$2}))
	{
	    $object = $1;
	    $action = $2;

	    $snd_action = "ACTION_$action";
	    $snd_action_default = $snd_action;
	}

	$token =~ s/\./_/g;
	$object =~ s/\./_/g;	# needed for "invisible_sand.active.digging"

	if ($object =~ /^\[(.+)\]$/)
	{
	    $object = 'CLASS_' . $1;
	}

	# print STDERR "'$token' => '$object', '$action'\n";

	my $full_element = "EL_$token";
	my $base_element = "EL_$object";

	my $element = $base_element;
	my $sound = $token;

	if ($sound =~ /^\[.+\]/)
	{
	    $sound =~ s/\[//;		# element class sound; begin ...
	    $sound =~ s/\]//;		# ... and end of definition token
	    $sound =~ s/^/CLASS_/;	# add class identifier
 	}

	$sound = "SND_$sound";

	my $element_without_action = $full_element;
	$element_without_action =~ s/_$action$//;

	my $element_class_without_action = $full_element;
	$element_without_action =~ s/_$action$//;

	if (defined($known_element_class{$full_element}))
	{
	    $element = $full_element;

	    $snd_action_default = '-1';
	}

	if ($element_without_action eq $element || $action eq '')
	{
	    $element_without_action = '';
	}

	if (defined($sound_without_action{$sound}))
	{
	    next;
	}

	if (!defined($known_element{$element}) &&
	    !defined($known_element_class{$element}))
	{
	    # print STDERR "----- ERROR: unknown element '$element' -----\n";

	    push @unknown_sounds, $sound;

	    next;
	}

	if (!($element =~ /_CLASS_/) &&
	    defined($known_element{$element}))
	{
	    print_element_to_sound_entry($element, "FALSE",
					 $snd_action_default,
					 $sound);
	}

	if (!($element =~ /_CLASS_/) &&
	    defined($known_element{$element_without_action}))
	{
	    print_element_to_sound_entry($element_without_action, "FALSE",
					 $snd_action,
					 $sound);
	}

	if ($element =~ /_CLASS_/ &&
	    defined($known_element_class{$element}))
	{
	    my $class_element = $known_element_class{$element};

	    print_element_to_sound_entry($class_element, "TRUE",
					 $snd_action_default,
					 $sound);
	}

	if ($element =~ /_CLASS_/ &&
	    defined($known_element_class{$element_without_action}))
	{
	    my $class_element = $known_element_class{$element_without_action};

	    print_element_to_sound_entry($class_element, "TRUE",
					 $snd_action,
					 $sound);
	}
    }

    print_element_to_sound_entry('-1', '-1', '-1', '-1');

    print "};\n";

    close FILE;

    if (scalar(@unknown_sounds) > 0)
    {
	print STDERR "-" x 79 . "\n";
	print STDERR "The following sounds cannot be associated with any element or element class:\n";

	foreach my $sound (@unknown_sounds)
	{
	    print STDERR "- $sound\n";
	}

	print STDERR "-" x 79 . "\n";
    }

    print_file_footer($filename_conf_e2s_c);
}

sub print_font_to_graphic_list
{
    my @unknown_graphics = ();
    my %known_font        = get_known_font_definitions();
    my %known_special_arg = get_known_special_arg_definitions();

    # ---------- read graphic file definitions ----------

    my $filename = "$src_path/conf_gfx.c";

    print_file_header($filename_conf_fnt_c, $text_fnt_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int font_nr;\n";
    print "  int special;\n";
    print "\n";
    print "  int graphic;\n";
    print "}\n";
    print "font_to_graphic[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (!contains_image_file($_))	# skip all lines without image file
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	my $token = $_;
	my $font;
	my $special;
	my $graphic;

	if ($token =~ /^(font\..*)$/)
	{
	    $font = $token;
	    $special = '-1';

	    if ($token =~ /^(.*)\.([^\.]+)$/ &&
		defined($known_special_arg{$2}))
	    {
		$font = $1;
		$special = "GFX_SPECIAL_ARG_" . $2;
	    }
	}
	else
	{
	    next;			# skip all non-font definitions
	}

	$token =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case
	$font  =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case

	$token =~ s/\./_/g;
	$font  =~ s/\./_/g;

	# print STDERR "'$token' => '$font'\n";

	$graphic = "IMG_$token";

	if (!defined($known_font{$font}))
	{
	    # print STDERR "----- ERROR: unknown font '$font' -----\n";

	    push @unknown_graphics, $graphic;

	    next;
	}

	print_font_to_graphic_entry($font,
				    $special,
				    $graphic);
    }

    print_font_to_graphic_entry('-1', '-1', '-1');

    print "};\n";

    close FILE;

    if (scalar(@unknown_graphics) > 0)
    {
	print STDERR "-" x 79 . "\n";
	print STDERR "The following graphics cannot be associated with any font:\n";

	foreach my $graphic (@unknown_graphics)
	{
	    print STDERR "- $graphic\n";
	}

	print STDERR "-" x 79 . "\n";
    }

    print_file_footer($filename_conf_fnt_c);
}

sub print_gamemode_to_sound_list
{
    my %known_prefix = get_known_sound_prefix_definitions();
    my %known_special_arg = get_known_special_arg_definitions();

    # ---------- read music file definitions ----------

    my $filename = "$src_path/conf_snd.c";

    print_file_header($filename_conf_g2s_c, $text_g2s_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int gamemode;\n";
    print "\n";
    print "  int sound;\n";
    print "}\n";
    print "gamemode_to_sound[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (!contains_sound_file($_))	# skip all lines without sound file
	{
	    next;
	}

	if (/\[not used\]/)
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	my $token = $_;
	my $gamemode = -1;
	# my $level = -1;		# ???

	if (defined($known_prefix{$token}))
	{
	    # no special arg defined
	}
	elsif ($token =~ /^(.*)\.([^\.]+)$/ &&
	       defined($known_prefix{$1}) &&
	       defined($known_special_arg{$2}))
	{
	    $gamemode = "GFX_SPECIAL_ARG_" . $2;
	}
	else
	{
	    next;
	}

#	if ($token =~ /^(.*)\./ &&
#	    !defined($known_prefix{$1}))
#	{
#	    next;
#	}
#
#	if ($token =~ /^(.*)\.([^\.]+)$/ &&
#	    defined($known_special_arg{$2}))
#	{
#	    $gamemode = "GFX_SPECIAL_ARG_" . $2;
#	}

	$token =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case
	$token =~ s/\./_/g;

	my $sound = "SND_$token";

	# print STDERR "'$token' => '$sound'\n";

	print_gamemode_to_sound_entry($gamemode, $sound);
    }

    print_gamemode_to_sound_entry('-1', '-1');

    print "};\n";

    close FILE;

    print_file_footer($filename_conf_g2s_c);
}

sub print_gamemode_to_music_list
{
    my %known_prefix = get_known_music_prefix_definitions();
    my %known_special_arg = get_known_special_arg_definitions();

    # ---------- read music file definitions ----------

    my $filename = "$src_path/conf_mus.c";

    print_file_header($filename_conf_g2m_c, $text_g2m_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "static struct\n";
    print "{\n";
    print "  int gamemode;\n";
    print "\n";
    print "  int music;\n";
    print "}\n";
    print "gamemode_to_music[] =\n";
    print "{\n";

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (!contains_music_file($_))	# skip all lines without music file
	{
	    next;
	}

	s/^  \{ \"//;			# cut all leading ...
	s/\",.*$//;			# ... and trailing garbage

	my $token = $_;
	my $gamemode = -1;

	if (defined($known_prefix{$token}))
	{
	    # no special arg defined
	}
	elsif ($token =~ /^(.*)\.([^\.]+)$/ &&
	       defined($known_prefix{$1}) &&
	       defined($known_special_arg{$2}))
	{
	    $gamemode = "GFX_SPECIAL_ARG_" . $2;
	}
	else
	{
	    next;
	}

#	my $prefix = $token;
#	$prefix =~ s/\..*$//;
#
##	if ($token =~ /^(.*)\./ &&
##	    !defined($known_prefix{$1}))
#	if (!defined($known_prefix{$prefix}))
#	{
#	    next;
#	}
#
#	if ($token =~ /^(.*)\.([^\.]+)$/ &&
#	    defined($known_special_arg{$2}))
#	{
#	    $gamemode = "GFX_SPECIAL_ARG_" . $2;
#	}

	$token =~ s/(.*)/uc($1)/eg;	# convert all characters to upper case
	$token =~ s/\./_/g;

	my $music = "MUS_$token";

	# print STDERR "'$token' => '$music'\n";

	print_gamemode_to_music_entry($gamemode, $music);
    }

    print_gamemode_to_music_entry('-1', '-1');

    print "};\n";

    close FILE;

    print_file_footer($filename_conf_g2m_c);
}

sub print_image_config_vars
{
    # ---------- read graphic file definitions ----------

    my $filename = "$src_path/conf_gfx.c";

    print_file_header($filename_conf_var_c, $text_var_c);

    open(FILE, "$filename") ||
	fail("cannot open file '$filename' for reading");

    print "struct TokenIntPtrInfo image_config_vars[] =\n";
    print "{\n";

    my $start_parsing = 0;

    while (<FILE>)
    {
	chomp;				# cut trailing newline

	if (/CONFIG_VARS_START/)	# keyword to start parsing file
	{
	    $start_parsing = 1;
	}

	if (!$start_parsing)
	{
	    next;
	}

	if (/^\s*\{\s*\"([^\"]+)\"/)	# config token found
	{
	    my $token = $1;
	    my $var = $token;

	    # --- some prefix replacements ---

	    $var =~ s/^main\./menu.main./;
	    $var =~ s/^\[player\]\./game.player_/;
	    $var =~ s/^\[title_initial\]/title_initial_default/;
	    $var =~ s/^\[title\]/title_default/;
	    $var =~ s/^\[titlescreen_initial\]/titlescreen_initial_default/;
	    $var =~ s/^\[titlescreen\]/titlescreen_default/;
	    $var =~ s/^\[titlemessage_initial\]/titlemessage_initial_default/;
	    $var =~ s/^\[titlemessage\]/titlemessage_default/;

	    if ($var =~ /^titlescreen.*(\d)/ ||
		$var =~ /^titlemessage.*(\d)/ ||
		$var =~ /^game.panel.key_(\d)/ ||
		$var =~ /^game.panel.inventory_first_(\d)/ ||
		$var =~ /^game.panel.inventory_last_(\d)/ ||
		$var =~ /^game.panel.conveyor_belt_(\d)\./ ||
		$var =~ /^game.panel.element_(\d)\./ ||
		$var =~ /^game.panel.graphic_(\d)\./ ||
		$var =~ /^game.panel.ce_score_(\d)\./)
	    {
		my $number = $1;
		my $array_pos = int($number) - 1;
		$var =~ s/_$number/\[$array_pos\]/;
	    }
	    elsif ($var =~ /^game.panel.conveyor_belt_(\d)(_switch)/ ||
		   $var =~ /^game.panel.element_(\d)(_count)/ ||
		   $var =~ /^game.panel.ce_score_(\d)(_element)/)
	    {
		my $number = $1;
		my $suffix = $2;
		my $array_pos = int($number) - 1;
		$var =~ s/_$number$suffix/$suffix\[$array_pos\]/;
	    }

	    # --- some suffix replacements ---

	    $var =~ s/^(menu\.main\..*)\.chars$/$1.size/;
	    $var =~ s/^(tape\.text\..*)\.chars$/$1.size/;
	    $var =~ s/^(game\.panel\..*)\.chars$/$1.size/;
	    $var =~ s/^(game\.panel\..*)\.tile_size$/$1.size/;
	    $var =~ s/^(request\.button\..*)\.tile_size$/$1.size/;
	    $var =~ s/\.digits$/.size/;
	    $var =~ s/\.2nd_offset$/.offset2/;
	    $var =~ s/\.2nd_xoffset$/.xoffset2/;
	    $var =~ s/\.2nd_yoffset$/.yoffset2/;
	    $var =~ s/\.element$/.id/;
	    $var =~ s/\.draw_order$/.sort_priority/;

	    $var =~ s/\.font_[a-z]+$/.font_alt/;
	    $var =~ s/\.INFO\[([A-Z]+)\]$/_info\[GFX_SPECIAL_ARG_INFO_$1\]/;
	    $var =~ s/\.SETUP\[([A-Z0-9_]+)\]$/_setup\[GFX_SPECIAL_ARG_SETUP_$1\]/;
	    $var =~ s/\.([A-Z]+)$/\[GFX_SPECIAL_ARG_$1\]/;
	    $var =~ s/\.([A-Z]+)\./\[GFX_SPECIAL_ARG_$1\]./;

	    if ($var =~ /^(menu.(enter|leave|next)_screen)(.[a-z_]+)$/)
	    {
		$var = $1 . "[GFX_SPECIAL_ARG_DEFAULT]" . $3;
	    }

	    if ($var =~ /^menu.(draw_[xy]offset|list_size)$/)
	    {
		$var .= "[GFX_SPECIAL_ARG_DEFAULT]";
	    }

	    if ($var =~ /^(viewport.(window|playfield|door_[12]))(.[a-z_]+)$/)
	    {
		$var = $1 . "[GFX_SPECIAL_ARG_DEFAULT]" . $3;
	    }

	    print_image_config_var_entry("\"$token\"", "&$var");

	    if ($var =~ /^(title)_default/ ||
		$var =~ /^(title_initial)_default/ ||
		$var =~ /^(titlescreen.*)\[\d\]/ ||
		$var =~ /^(titlemessage.*)\[\d\]/)
	    {
		my $prefix = $1;
		$var =~ s/^$prefix/${prefix}_first/;

		print_image_config_var_entry("\"$token\"", "&$var");
	    }
	}
    }

    print_image_config_var_entry('NULL', 'NULL');

    print "};\n";

    close FILE;

    print_file_footer($filename_conf_var_c);
}

sub print_active_states
{
    # ---------- read graphic file definitions ----------

    my %known_element = get_known_element_definitions();
    my %known_button  = get_known_button_definitions();
    my %known_font    = get_known_font_definitions();

    print_file_header($filename_conf_act_c, $text_act_c);

    print "static struct\n";
    print "{\n";
    print "  int element;\n";
    print "  int element_active;\n";
    print "}\n";
    print "element_with_active_state[] =\n";
    print "{\n";

    foreach my $element (sort keys %known_element)
    {
	my $element_active = $element . '_ACTIVE';

	if (defined($known_element{$element_active}))
	{
	    print_active_state_entry($element, $element_active);
	}
    }

    print_active_state_entry('-1', '-1');

    print "};\n";

    print "\n";
    print "\n";
    print "static struct\n";
    print "{\n";
    print "  int button;\n";
    print "  int button_active;\n";
    print "}\n";
    print "button_with_active_state[] =\n";
    print "{\n";

    foreach my $button (sort keys %known_button)
    {
	my $button_active = $button . '_ACTIVE';

	if (defined($known_button{$button_active}))
	{
	    print_active_state_entry($button, $button_active);
	}
    }

    print_active_state_entry('-1', '-1');

    print "};\n";

    print "\n";
    print "\n";
    print "static struct\n";
    print "{\n";
    print "  int font_nr;\n";
    print "  int font_nr_active;\n";
    print "}\n";
    print "font_with_active_state[] =\n";
    print "{\n";

    foreach my $font (sort keys %known_font)
    {
	my $font_active = $font . '_ACTIVE';

	if (defined($known_font{$font_active}))
	{
	    print_active_state_entry($font, $font_active);
	}
    }

    print_active_state_entry('-1', '-1');

    print "};\n";

    print_file_footer($filename_conf_act_c);
}


# =============================================================================
# M A I N - P R O G R A M
# =============================================================================

sub main
{
    my $argc = scalar(@ARGV);

    if ($argc == 0 || $ARGV[0] eq '-h' || $ARGV[0] eq '--help')
    {
	print "Usage: $0 <file>\n\n";
	print "Choose <file> from the following list:\n";
	print "- '$filename_conf_gfx_h'\n";
	print "- '$filename_conf_snd_h'\n";
	print "- '$filename_conf_mus_h'\n";
	print "- '$filename_conf_chr_c'\n";
	print "- '$filename_conf_chr_h'\n";
	print "- '$filename_conf_cus_c'\n";
	print "- '$filename_conf_cus_h'\n";
	print "- '$filename_conf_grp_c'\n";
	print "- '$filename_conf_grp_h'\n";
	print "- '$filename_conf_e2g_c'\n";
	print "- '$filename_conf_esg_c'\n";
	print "- '$filename_conf_fnt_c'\n";
	print "- '$filename_conf_g2s_c'\n";
	print "- '$filename_conf_g2m_c'\n";
	print "- '$filename_conf_var_c'\n";
	print "- '$filename_conf_act_c'\n";

	exit 1;
    }

    if ($ARGV[0] eq $filename_conf_gfx_h)
    {
	print_graphics_list();
    }
    elsif ($ARGV[0] eq $filename_conf_snd_h)
    {
	print_sounds_list();
    }
    elsif ($ARGV[0] eq $filename_conf_mus_h)
    {
	print_music_list();
    }
    elsif ($ARGV[0] eq $filename_conf_chr_c)
    {
	print_chars_graphics_list();
    }
    elsif ($ARGV[0] eq $filename_conf_chr_h)
    {
	print_chars_elements_list();
    }
    elsif ($ARGV[0] eq $filename_conf_cus_c)
    {
	print_custom_graphics_list();
    }
    elsif ($ARGV[0] eq $filename_conf_cus_h)
    {
	print_custom_elements_list();
    }
    elsif ($ARGV[0] eq $filename_conf_grp_c)
    {
	print_group_graphics_list();
    }
    elsif ($ARGV[0] eq $filename_conf_grp_h)
    {
	print_group_elements_list();
    }
    elsif ($ARGV[0] eq $filename_conf_e2g_c)
    {
	print_element_to_graphic_list();
    }
    elsif ($ARGV[0] eq $filename_conf_esg_c)
    {
	print_element_to_special_graphic_list();
    }
    elsif ($ARGV[0] eq $filename_conf_e2s_c)
    {
	print_element_to_sound_list();
    }
    elsif ($ARGV[0] eq $filename_conf_fnt_c)
    {
	print_font_to_graphic_list();
    }
    elsif ($ARGV[0] eq $filename_conf_g2s_c)
    {
	print_gamemode_to_sound_list();
    }
    elsif ($ARGV[0] eq $filename_conf_g2m_c)
    {
	print_gamemode_to_music_list();
    }
    elsif ($ARGV[0] eq $filename_conf_var_c)
    {
	print_image_config_vars();
    }
    elsif ($ARGV[0] eq $filename_conf_act_c)
    {
	print_active_states();
    }
    else
    {
	print "Unknown option '$ARGV[0]'.\n";

	exit 1;
    }

    exit 0;
}
