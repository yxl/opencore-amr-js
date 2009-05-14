@rem = 'Perl, ccperl read this as an array assignment & skip the goto
@echo off
goto endofperl
@rem ';
#!/usr/atria/bin/Perl
#----------------------------------------------------------------------
#  Copyright (C) 2003 PacketVideo
#  This script is to automate the building of API Documentation
#  through use of the Doxygen tool.
#
#  See http://www.stack.nl/~dimitri/doxygen/  for more information on
#  Doxygen.
#
# ---------------------------------------------------------------------

use Cwd;
use Getopt::Long;
use Time::Local;
use File::Basename;

my $ScriptName = $0;
my $date_str;
my $DATE_STR;
my $doctype_list_string;
my @doc_list;
my $level_string;
my $PostDocs;
my $InputVersion;
my $help ;
my $DS;
my $VIEW;
my $TMPDIR;
my $DOXYGEN;
my $INDEXFILE;
my $BUILDNUM;
my $LBUILDNUM;
my $CFGFILE="config.doxy";
my $LOGFILE;
my $BUILDDIR;
my $STAGEDIR;
my $DOCDIR;
my $CC_DIR;
my $DOC_TYPE;
my $PROJ_NAME;
my $INPUTDIR;
my $WORDLIST;
my $FILETYPE;
my $EXCLUDE;
my $EXCLUDE_PATTERN;
my $CurrDir = cwd();
my $OldPath = $ENV{"PATH"};
my $MOVED = 0;
my $AddCopyright = 0;

# Graphic Conversion Variables
my $CONVERSIONHOST="winbuild05";
my $CONVERSIONDRIVE = "C:";
my $CONVERSIONDIR = "DOCUMENT_CONVERSION";
my $CONVERSIONTIMER = 10;
my $LOCKFILE = "convert.lock";
my $MS_VISIO = "${CONVERSIONDRIVE}\\Program Files\\Microsoft Office\\Visio11\\Visio.exe";
my $MIKTEX_PATH = "${CONVERSIONDRIVE}\\miktex\\miktex\\bin";
my $VISIOTOJPG = "${CONVERSIONDRIVE}\\doc_tools\\VisioToJpg.exe";
my $WORDTOPDF = "${CONVERSIONDRIVE}\\doc_tools\\Convert2PDF.bat";
my $RCP;


{
    local(@whoami) = split('/', $0);
    $whoami = pop(@whoami);
};

sub usage {

    print STDERR "\nUsage: doc_build [--help] [--doctype <list of documents> ]\n";
    print STDERR "[--path <path to documents>][--word <list of files to convert>]\n";
    print STDERR "[--filetype <file types>][--exclude <files/directory>]\n";
    print STDERR "[--exclude_pattern <files/directory>][--title <title of document>]\n";
    print STDERR "[--level <documentation level>][--ver <version to specify>]\n";
    print STDERR "[--post ][--copyright]\n\n";

    print STDERR "The options are:\n";

    print STDERR "[--doctype <list of documents>] specify the documents to build.  Default:  All\n";
    print STDERR "[--title <title of documents>] title to use in the document.\n";
    print STDERR "[--path <path to documents>] specify the path documents to build if not one of the standard documents/directories.\n";
    print STDERR "[--word <list of files to convert>] specify files to convert to PDF format\n"; 
    print STDERR "[--filetype <file types>] default: *.h *.cpp\n";
    print STDERR "[--exclude <files/directory>] default:  none\n";
    print STDERR "[--exclude_pattern <files/directory>] default:  none\n";
    print STDERR "[--ver <version>] version to use in the document.\n";
    print STDERR "[--level <list of documents>] specifiy document level, default: All\n";
    print STDERR "[--post] Post documents on website\n";
    print STDERR "[--copyright] Add copyright to front page of document\n";
    print STDERR "[--help] Display usage\n";

    print STDERR "\n";
    exit (-1);
}

sub trim {
	my @out = @_;

	for (@out) {
		s/^\s+//;
		s/\s+$//;
	}

	return wantarray ? @out : $out[0];
}

sub GenerateDatestr {

	my $type = $_[0];
	my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);
	my $datestring;
	# year has 1900 subtracted from it
	$year += 1900;
	# only want the last two digits from the year
	$year =~ s/\d{2}//;
	# mon is zero-based
	$mon++;
	if ( $type eq "time" ) {
		$datestring = sprintf "%02d%02d%02d%02d%02d%02d\n", $year, $mon, $mday, $hour, $min, $sec;
	} elsif ( $type eq "string" ) {
		$datestring = scalar(localtime(time));
		my @date = split(/ +/, $datestring);
		my $month;
		SWITCH: {
			if ( $date[1] =~ /Jan/ ) { $month = "January"; last SWITCH; }
			if ( $date[1] =~ /Feb/ ) { $month = "February"; last SWITCH; }
			if ( $date[1] =~ /Mar/ ) { $month = "March"; last SWITCH; }
			if ( $date[1] =~ /Apr/ ) { $month = "April"; last SWITCH; }
			if ( $date[1] =~ /May/ ) { $month = "May"; last SWITCH; }
			if ( $date[1] =~ /Jun/ ) { $month = "Jun"; last SWITCH; }
			if ( $date[1] =~ /Jul/ ) { $month = "July"; last SWITCH; }
			if ( $date[1] =~ /Aug/ ) { $month = "August"; last SWITCH; }
			if ( $date[1] =~ /Sep/ ) { $month = "September"; last SWITCH; }
			if ( $date[1] =~ /Oct/ ) { $month = "October"; last SWITCH; }
			if ( $date[1] =~ /Nov/ ) { $month = "November"; last SWITCH; }
			if ( $date[1] =~ /Dec/ ) { $month = "December"; last SWITCH; }
			$nothing = 1;
		}

		$datestring = "$month $date[2], $date[4]";
	} else {
		$datestring = sprintf "%02d%02d%02d\n", $year, $mon, $mday;
	}
	chomp ($datestring);
	return ($datestring);
}

sub SetView {

	if ( $VIEW eq "" ) {
		# On Windows, cleartool -pwv doesn't work in all view types.
		# So determine VIEW by path used to get to script.
		# If still can't find it, prompt for view.
		my $dir = dirname($ScriptName);
		
		chdir("$dir");
	
		chdir("../../../..");
		$VIEW = cwd();
		if ($ENV{'OS'} =~ /Windows/) {
			$VIEW =~ s/\///g;
		}
		chomp ($VIEW);

		if ( $VIEW eq "" || ! -e "${VIEW}${DS}extern_tools_v2" ) {
			print "Unable to determine view to use.\n";
			print "Please enter view path to use:  ";
			$VIEW = <STDIN>;
			chomp ($VIEW);
		}
	
		# Remove any trailing slashes.
		if ( $VIEW =~ /${DS}\$/ ) {
			chop($VIEW);
		}
	}

	if ( ! -e "${VIEW}${DS}extern_tools_v2" ) {
		if ( -e "${VIEW}${DS}vobs${DS}extern_tools_v2" ) {
			$VIEW = "${VIEW}${DS}vobs";
		} else {
			print "Unable to find $VIEW${DS}extern_tools_v2.\n\n";
			exit(-1);
		}
	}

	print "VIEW is set to $VIEW\n";
}

sub GetBuildNum {

	my $BuildConfigSpec="${VIEW}${DS}pvadmin${DS}BranchConfigSpec.txt";
	my $Label;
	my @list = ();

	# If version label was passed in, use that.
	# Next, try to get label from Config Spec.
	# Next, try to get label from Clearcase elements.
	# If none of the files are in Clearcase, create a generic build label.
	if ( $InputVersion ne "" ) {
		$BUILDNUM="$InputVersion";
		return (0);
	}

	chdir("${VIEW}${DS}pvadmin");
	$status = `cleartool catcs`;
	@list = split(/\n/, $status);
	foreach $line (@list) {
		if ( !( $line =~ /^#/ )) {
			if ( !( $line =~ /^$/ )) {
				if ( $Label eq "" ) {
					$Label = $line;
					chomp($Label);
				}
			}
		}
	}

	if ( $Label =~ /element/ ) {
		@list = split(/ /,$Label);
		$Label = "$list[$#list]";
	} elsif ( $Label =~ /include/ ) {
		@list = split(/ /,$Label);
		$Label = "$list[$#list]";
		@list = split(/\//,$Label);
		$Label = "$list[$#list]";
	}


	if ( ( $Label =~ /LATEST/ ) || ($Label =~ /CHECKEDOUT/ ) ) {
		$Label = "";
		open(FIN, "$BuildConfigSpec");
		while (<FIN>) {
			if ( !( $_ =~ /^#/ )) {
				if ( !( $_ =~ /^$/ )) {
					if ( $Label eq "" ) {
						$Label = $_;
						chomp($Label);
					}
				}
			}
		}
		close FIN;

		if ( $Label =~ /element/ ) {
			@list = split(/ /,$Label);
			$Label = "$list[$#list]";
		} elsif ( $Label =~ /include/ ) {
			@list = split(/ /,$Label);
			$Label = "$list[$#list]";
		}

	}

	$BUILDNUM = $Label;
}

sub SetEnv {

	$date_str = GenerateDatestr();
	$DATE_STR = GenerateDatestr("string");

	$INDEXFILE="/opt/pvserver/web/webapps/docs/index.html";

	# Create directory split variable
	$DS = "/";

	if ($ENV{'OS'} =~ /Windows/) {
		fileparse_set_fstype("MSDOS");
		# Create directory split variable
		$DS = "\\";
	}
	  # see if caller knows where we should put temps
	  # if not, $TMPDIR will stay undef
	$TMPDIR=$ENV{'DOC_BUILD__TMPDIR'};

        # see if caller wants to override VIEW
	$VIEW=$ENV{'VIEW'};

	if ($ENV{'OS'} =~ /Windows/) {
		# Create platform specific commands
		$RCP = "rcp -b";
		$MKDIR = "mkdir";
		$RMDIR = "rmdir /S /Q";
		$COPY = "copy";
	
		# Create platform specific Environment
		$TMPDIR = "C:\\temp" if !$TMPDIR;
		$BUILDDIR="${TMPDIR}\\documentation_build";
		$STAGEDIR="${TMPDIR}\\documentation_stage";
		$LOGFILE="${TMPDIR}\\docbuild.log";
		$status = `del /Q $LOGFILE 2>&1`;
	} else {
		# Create platform specific commands
		$RCP = "rcp";
		$MKDIR = "mkdir -p";
		$RMDIR = "rm -rf";
		$COPY = "cp";
	
		# Create platform specific Environment
		$TMPDIR = "/tmp" if !$TMPDIR;
		$BUILDDIR="${TMPDIR}/documentation_build";
		$STAGEDIR="${TMPDIR}/documentation_stage";
		$LOGFILE="${TMPDIR}/docbuild.log";
		$status = `rm -f $LOGFILE 2>&1`;
	}
}

sub SetCCEnv {

	SetView();
	# Once we know path to view, include PVutilities.pl
	if ($ENV{OS} =~ /Windows/) {
		$lib_path = "$VIEW\\tools_v2\\build\\document\\bin";
	} else {
		$lib_path = "$VIEW/tools_v2/build/document/bin";
	}
	# DEBUG
#	$lib_path = "/tmp/doc";
	unshift (@INC, "$lib_path");
	require "PVutilities.pl";

	GetBuildNum();
	print "Using build label: $BUILDNUM.\n";

	# LBUILDNUM is for Latex files - need different format.
	$LBUILDNUM="$BUILDNUM";
	$LBUILDNUM =~ s/_/\\_/g;

	# Create platform specific Environment
	if ($ENV{'OS'} =~ /Windows/) {
		$DOXYGEN="${VIEW}\\extern_tools_v2\\doxygen\\win32\\doxygen.exe";
		$CC_DIR="${VIEW}\\tools_v2\\build\\document\\templates\\";
		$ENV{"PATH"} = "${VIEW}\\extern_tools_v2\\doxygen\\win32;${OldPath}";
	} else {
		$SYSTYPE = `uname -s`;
		chomp ($SYSTYPE);
		if ($SYSTYPE =~ /SunOS/) {
			$DOXYGEN="${VIEW}/extern_tools_v2/doxygen/sun/doxygen";
		} elsif ($SYSTYPE =~ /Linux/) {
			$DOXYGEN="${VIEW}/extern_tools_v2/doxygen/linux/doxygen";
		} elsif ($SYSTYPE =~ /HP-UX/) {
			$DOXYGEN="${VIEW}/extern_tools_v2/doxygen/hpux/doxygen";
		} else {
			print STDERR "Unknown system type $SYSTYPE\n\n";
			exit (-1);
		}
		$CC_DIR="${VIEW}/tools_v2/build/document/templates/";

		# Setup Path so we find latex files
		$ENV{"PATH"} = "${VIEW}/extern_tools_v2/latex/linux/bin:${OldPath}";
		$ENV{"TETEXDIR"} = "${VIEW}/extern_tools_v2/latex/linux/share/web2c";
		$ENV{"TEXMFMAIN"} = "${VIEW}/extern_tools_v2/latex/linux/share";
	}
}

sub ConvertFilename {

	$_ = $_[0];
	if ($ENV{'OS'} =~ /Windows/) {
		tr/\//\\/;
	} else {
		tr/\\/\//;
	}

	return $_;
}

sub GetDirname {

	my $filename = $_[0];
	my $dir;

	if ( $filename =~ /\// ) {
		fileparse_set_fstype("UNIX");
	} else {
		fileparse_set_fstype("MSDOS");
	}

	$dir = dirname($filename);

	if ( $dir =~ /\// ) {
		$dir = "${dir}/";
	} else {
		$_ = $dir;
		s/\\/\\\\/g;
		$dir = $_;
		$dir = "${dir}\\\\";
	}

	# Reset to proper platform
	if ($ENV{'OS'} =~ /Windows/) {
		fileparse_set_fstype("MSDOS");
	} else {
		fileparse_set_fstype("UNIX");
	}

	return $dir;
}

sub CreateConversionLock {

	my $Hostname = `hostname`;
	chomp $Hostname;

	# No longer need to do this with VisioToJpg tool
        # Check that someone is logged in to ${CONVERSIONHOST}
        # This is required for RSHD to pass keystrokes to Windows applications.
        # 'qwinsta' can be used to query currently logged in users.
       	# $status = `rsh ${CONVERSIONHOST} "qwinsta"`;
	#      	if ( $status =~ /Active/ ) {
	# 		# Good - found active user
	# 	} else {
	#                # Assume nobody is not logged in.
	#                print "\nWarning:  A user does not appear to be logged in to Conversion server ${CONVERSIONHOST}.\n\n";
	#                print "This is necessary for document conversion.\n";
	#                print "Please login on ${CONVERSIONHOST} and restart script.\n\n";
	#		exit(-1);
	#        }

	my $lockmessage = "Locked on $DATE_STR from $Hostname";
	my $status = `rsh $CONVERSIONHOST \"DIR ${CONVERSIONDRIVE}\\${LOCKFILE}\ 2>&1\"`;
	if ($?) {
		print "rsh failed to connect to host $CONVERSIONHOST\n";
		return (1);
	} else
	{
        	if ( $status =~ /File Not Found/ )
		{
			# create lock file
			$status = `rsh ${CONVERSIONHOST} "ECHO ${lockmessage} > ${CONVERSIONDRIVE}\\${LOCKFILE}"`;
		} else
		{
			$status = `rsh ${CONVERSIONHOST} "TYPE ${CONVERSIONDRIVE}\\${LOCKFILE}"`;
			print "Conversion server ${CONVERSIONHOST} may be in use\n\n";
			print "$status\n";
	                print "Check for ${CONVERSIONDRIVE}\\${LOCKFILE} file\n\n";
			return (1);
		}
	}
}

sub SetupConverter {

	my $status;
	# create temporary directory for conversion
	$status = `rsh ${CONVERSIONHOST} "RMDIR /S /Q ${CONVERSIONDRIVE}\\${CONVERSIONDIR} 2>&1"`;
	if ($?) {
		print "rsh failed to remove ${CONVERSIONDIR} from $CONVERSIONHOST\n";
		return (1);
	}

	$status = `rsh ${CONVERSIONHOST} "MKDIR ${CONVERSIONDRIVE}\\${CONVERSIONDIR} 2>&1"`;
	if ($?) {
		print "rsh failed to create ${CONVERSIONDIR} from $CONVERSIONHOST\n";
		return (1);
	}

	$status = `rsh ${CONVERSIONHOST} "MKDIR ${CONVERSIONDRIVE}\\temp 2>&1"`;
}

sub TransferFiles {

	my $status;
	my $file_to_convert=$_[0];

#	print ">>>> Transferring $file_to_convert to ${CONVERSIONHOST}.\n";
	if ($ENV{'OS'} =~ /Windows/) {
		$filedir = dirname($file_to_convert);
		$file_to_convert = basename($file_to_convert);
		chdir("${filedir}");
		$status = `$RCP ${file_to_convert} ${CONVERSIONHOST}:\\${CONVERSIONDIR}`;
	} else {
		$status = `$RCP ${file_to_convert} ${CONVERSIONHOST}:\\\\${CONVERSIONDIR}`;
	}

	if ($?) {
		print "Failed to copy ${file_to_convert} to $CONVERSIONHOST\n";
		return (1);
	}

}

sub RunConversion {

	my $convert_file = $_[0];
	my $output_file = $_[1];
	my $page_name = $_[2];

	if ( $page_name eq "" ) {
		$command = "$VISIOTOJPG -vsd ${CONVERSIONDRIVE}\\\\${CONVERSIONDIR}\\\\${convert_file} -jpg ${CONVERSIONDRIVE}\\\\${CONVERSIONDIR}\\\\${output_file}";
	} else {
		$command = "$VISIOTOJPG -vsd ${CONVERSIONDRIVE}\\\\${CONVERSIONDIR}\\\\${convert_file} -jpg ${CONVERSIONDRIVE}\\\\${CONVERSIONDIR}\\\\${output_file} -page \\\"${page_name}\\\"";
	}

	$status = `rsh ${CONVERSIONHOST} "$command" "$MS_VISIO"`;
}

sub RunWordToPDFConversion {

	my $convert_file = $_[0];

	$command = "$WORDTOPDF ${CONVERSIONDRIVE}\\\\${CONVERSIONDIR}\\\\${convert_file}";

	$status = `rsh ${CONVERSIONHOST} "$command"`;
}

sub RetrieveFiles {

	my $status;
	my $file_to_retrieve=$_[0];
	my $OldPwd = cwd();

#	print ">>>> Retrieving $file_to_retrieve from ${CONVERSIONHOST}.\n";

	chdir("${DOCDIR}");
	if ($ENV{'OS'} =~ /Windows/) {
		$status = `$RCP ${CONVERSIONHOST}:\\${CONVERSIONDIR}\\${file_to_retrieve} html${DS}`;
		$status = `$RCP ${CONVERSIONHOST}:\\${CONVERSIONDIR}\\${file_to_retrieve} latex${DS}`;
	} else {
		$status = `$RCP ${CONVERSIONHOST}:\\\\${CONVERSIONDIR}\\\\${file_to_retrieve} ${DOCDIR}${DS}html`;
		$status = `$RCP ${CONVERSIONHOST}:\\\\${CONVERSIONDIR}\\\\${file_to_retrieve} ${DOCDIR}${DS}latex`;
	}
	chdir("${OldPwd}");

	if ($?) {
		print "Failed to retrieve ${file_to_retrieve} from $CONVERSIONHOST\n";
		return (1);
	}

}

sub RetrieveWordFiles {

	my $status;
	my $file_to_retrieve=$_[0];
	my $OldPwd = cwd();

#	print ">>>> Retrieving $file_to_retrieve from ${CONVERSIONHOST}.\n";

	chdir("${DOCDIR}");
	if ($ENV{'OS'} =~ /Windows/) {
		$status = `$RCP ${CONVERSIONHOST}:\\${CONVERSIONDIR}\\${file_to_retrieve} .`;
	} else {
		$status = `$RCP ${CONVERSIONHOST}:\\\\${CONVERSIONDIR}\\\\${file_to_retrieve} ${DOCDIR}`;
	}
	chdir("${OldPwd}");

	if ($?) {
		print "Failed to retrieve ${file_to_retrieve} from $CONVERSIONHOST\n";
		return (1);
	}

}

sub RemoveLock {
	my $status = `rsh $CONVERSIONHOST \"DEL ${CONVERSIONDRIVE}\\${LOCKFILE}\ 2>&1"`;
	if ($?) {
		print "rsh failed to remove convert.lock from $CONVERSIONHOST\n";
		return (1);
	}
}

sub ConvertVisio {

	my $visio_filename = $_[0];
	my $visio_pagename = $_[1];
	my $output_filename;
	
	$output_filename = basename($visio_filename);
        @output_name = split(/\./, $output_filename);
        $output_filename = "$output_name[$0].jpg";

	print "     >>>>> Converting Visio file: $visio_filename\n";

	if ( CreateConversionLock() != 0 ) {
		print "Unable to convert $visio_filename\n";
		return "";
	}

	if ( SetupConverter() != 0 ) {
		print "Unable to convert $visio_filename\n";
		RemoveLock();
		return "";
	}
	
	if ( TransferFiles($visio_filename) != 0 ) {
		RemoveLock();
		exit(-1);
	}

        @visio_name = split(/\./, $output_filename);
        $visio_filename = "$visio_name[$0].vsd";

	if ( RunConversion($visio_filename, $output_filename, $visio_pagename) != 0 ) {
		RemoveLock();
		exit(-1);
	}

	if ( RetrieveFiles($output_filename) != 0 ) {
		RemoveLock();
		exit(-1);
	}


	RemoveLock();
	return $output_filename;

}

sub ConvertWordToPDF {

	my $word_filename = $_[0];
	my $output_filename;
	
	$output_filename = basename($word_filename);
	@output_name = split(/\./, $output_filename);
	$output_filename = "$output_name[$0].pdf";
	$buildtype = "$output_name[$0]";

	$DOCDIR="${BUILDDIR}${DS}${buildtype}";
	$status = `${RMDIR} $DOCDIR >> $LOGFILE 2>&1`;
	$status = `${MKDIR} $DOCDIR`;
	if ($ENV{'OS'} !~ /Windows/) {
		$status = `chmod 777 $BUILDDIR`;
		$status = `chmod 777 $DOCDIR`;
	}

	print "     >>>>> Converting Word file to PDF: $word_filename\n";

	if ( CreateConversionLock() != 0 ) {
		print "Unable to convert $word_filename\n";
		return "";
	}

	if ( SetupConverter() != 0 ) {
		print "Unable to convert $word_filename\n";
		RemoveLock();
		return "";
	}
	
	if ( TransferFiles($word_filename) != 0 ) {
		RemoveLock();
		exit(-1);
	}

	if ( RunWordToPDFConversion(basename($word_filename)) != 0 ) {
		RemoveLock();
		exit(-1);
	}

	if ( RetrieveWordFiles($output_filename) != 0 ) {
		RemoveLock();
		exit(-1);
	}

	RemoveLock();

	print ">>>>> PDF File is located in ${DOCDIR}\n\n";
}

sub Grep {

	my $returnlines;
	my $pattern = $_[0];
	my $file = $_[1];

	$grepstatus = `find "$pattern" $file`;
        my @name = split(/\\/, $file);
        $grepfname = basename($file);
	$Fname = uc($grepfname);

	foreach $grepentry ( split(/\n/, $grepstatus) ) {
		if ( $grepentry eq "" ) {
			next;
		}
		if ( $grepentry =~ /$Fname/ )  {
			next;
		}
		$returnlines = "$returnlines $grepentry\n";
	}

	return $returnlines;
}

sub UpdateHtml {

	my $file = $_[0];
	my $oldfile = $_[1];
	my $newfile = $_[2];
	my $scale = $_[3];
	my $page = $_[4];

	if ( $oldfile =~ /\\/ ) {
		$_ = $oldfile;
		s/\\/\\\\/g;
		$oldfile = $_;
	}

	ReplaceString( -all, "$file", "INCLUDEVISIO", "\\|", "");
	ReplaceString( -all, "$file", "$oldfile", "$oldfile", "$newfile");
	ReplaceString( -all, "$file", "INCLUDEVISIO", "\{", "\"");
	ReplaceString( -all, "$file", "INCLUDEVISIO", "\}", "\">");
	ReplaceString( -all, "$file", "INCLUDEVISIO", "INCLUDEVISIO", "<img src");

	if ( $scale ne "" ) {
        	ReplaceString ( -all, "$file", "${newfile}${scale}", "${newfile}${scale}", "${newfile}");
	}
	if ( $page ne "" ) {
		ReplaceString ( -all, "$file", "${newfile}${page}", "${page}", "");
	}
}

sub UpdateLatexFormat {

	
	my @latex_files;
	my $lfiles;

	chdir("${DOCDIR}${DS}latex");
	@latex_files = <*.tex>;
	foreach $lfile ( @latex_files ) {

		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", '\$', "");
		ReplaceString( -all, "$lfile", "INCLUDEVISIO", "\\|", "");
		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", 'backslash', "");
		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", "\\\\_\\\\-", "_");
		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", "\\\\-", "");
		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", "\\\\{", "{");
		ReplaceString ( -all, "$lfile", "INCLUDEVISIO", "\\\\}", "}");
	}
}

sub UpdateLatexFile {

        my $file = $_[0];
        my $oldfile = $_[1];
        my $newfile = $_[2];
        my $scale = $_[3];
        my $page = $_[4];
	my $dir = GetDirname($oldfile);

        ReplaceString ( -all, "$file", "$dir", "$dir", "");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", ".vsd", ".jpg");

	if ( $scale eq "" ) {
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO=", "\\includegraphics[scale=1]"); 
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO = ", "\\includegraphics[scale=1]"); 
	} else {
		ReplaceString ( -all, "$file", "${newfile}${scale}", "INCLUDEVISIO=", "\\includegraphics[scale=$scale]"); 
		ReplaceString ( -all, "$file", "${newfile}${scale}", "INCLUDEVISIO = ", "\\includegraphics[scale=$scale]"); 
        	ReplaceString ( -all, "$file", "${newfile}${scale}", "${newfile}${scale}", "${newfile}");
	}
	if ( $page ne "" ) {
		ReplaceString ( -all, "$file", "${newfile}${page}", "${page}", "");
	}
}

sub UpdateLatex {

        my $file = $_[0];
        my $oldfile = $_[1];
        my $newfile = $_[2];
        my $scale = $_[3];
        my $page = $_[4];
	my $dir = GetDirname($oldfile);

	ReplaceString ( -all, "$file", "INCLUDEVISIO", '\$', "");
	if ( $scale ne "" ) {
		ReplaceString( -all, "$file", "INCLUDEVISIO", "$scale", "");
		ReplaceString( -all, "$file", "$newfile", "$scale", "");
	}
	if ( $page ne "" ) {
		ReplaceString( -all, "$file", "INCLUDEVISIO", "$page", "");
		ReplaceString( -all, "$file", "$newfile", "$page", "");
	}
	ReplaceString( -all, "$file", "INCLUDEVISIO", "\\|", "");
	ReplaceString ( -all, "$file", "INCLUDEVISIO", 'backslash', "");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", "\\\\_\\\\-", "_");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", "\\\\-", "");
        ReplaceString ( -all, "$file", "$dir", "$dir", "");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", ".vsd", ".jpg");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", "\\\\{", "{");
        ReplaceString ( -all, "$file", "INCLUDEVISIO", "\\\\}", "}");
	if ( $scale eq "" ) {
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO=", "\\includegraphics[scale=1]"); 
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO = ", "\\includegraphics[scale=1]"); 
	} else {
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO=", "\\includegraphics[scale=$scale]"); 
		ReplaceString ( -all, "$file", "$newfile", "INCLUDEVISIO = ", "\\includegraphics[scale=$scale]"); 
	}
}

sub InstallGraphics {

	my $status;
	my $file_list;
	my $page_name;
	my $jpg_scale;
	my $extension;

	print "     >>>>> Checking for graphic files to install.\n";

	if ($ENV{'OS'} =~ /Windows/) {
		$status = `dir /A-D /B /S $INPUTDIR`; 
	} else {
		$status = `find $INPUTDIR -type f -print`;
	}
	@file_list = split(/\n/,$status);
	foreach $fname (@file_list) {

		# Check that file is a code file
		@file_part = split(/\./, $fname);
		$extension = $file_part[1];
		if ( ( $extension ne "h" ) && ( $extension ne "cpp" ) ) {
			next;
		}

		# Check that file exists
		if ( ! -e $fname ) {
			next;
		}

		# Check for latex graphics
		if ($ENV{'OS'} =~ /Windows/) {
			$status = Grep("includegraphic",$fname);
		} else {
			$status = `grep includegraphic $fname`;
		}
		chomp $status;
		if ( $status ne "" ) {
			foreach $fentry ( split(/\n/, $status) ) {
				@fields = split(/[{}]/,$fentry);
				$filename = $fields[1];
				$latex_dir = GetDirname($filename);
				if ( ! -e $filename ) {
					$filename = ConvertFilename($filename);
					if ( -e $filename ) {
						# filename just needed converting
					} elsif ( -e "${VIEW}${filename}" ) {
						$filename = "$VIEW$filename";
					} elsif ( -e "${VIEW}${DS}${filename}" ) {
						$filename = "$VIEW${DS}$filename";
					} else {
						# If file is still not found, print error.
						print STDERR "ERROR: $filename cannot be located\n";
						exit();
					}
				}
		
				$status = `${COPY} $filename ${DOCDIR}${DS}latex`;
				chdir("${DOCDIR}${DS}latex");
				@latex_files = <*.tex>;
				foreach $lfile ( @latex_files ) {
        				ReplaceString ( -all, "${lfile}", "\\includegraphics", "${latex_dir}", "");
				}
			}
		}
	
		# Check for html graphics
		if ($ENV{'OS'} =~ /Windows/) {
			$status = Grep("img src",$fname);
		} else {
			$status = `grep "img src" $fname`;
		}
		chomp $status;
		if ( $status ne "" ) {
			foreach $fentry ( split(/\n/, $status) ) {
				@fields = split(/["]/,$fentry);
				$filename = $fields[1];
				$html_dir = GetDirname($filename);
				if ( ! -e $filename ) {
					$filename = ConvertFilename($filename);
					if ( -e $filename ) {
						# filename just needed converting
					} elsif ( -e "${VIEW}${filename}" ) {
						$filename = "$VIEW$filename";
					} elsif ( -e "${VIEW}${DS}${filename}" ) {
						$filename = "$VIEW${DS}$filename";
					} else {
						# If file is still not found, print error.
						print STDERR "ERROR: $filename cannot be located\n";
						exit();
					}
				}
				$status = `${COPY} $filename ${DOCDIR}${DS}html`;
				chdir("${DOCDIR}${DS}html");
				@html_files = <*.html>;
				foreach $hfile ( @html_files ) {
					ReplaceString ( -all, "${hfile}", "img src", "${html_dir}", "");
				}
			}
		}

		# Check for Visio graphics
		if ($ENV{'OS'} =~ /Windows/) {
			$status = Grep("INCLUDEVISIO",$fname);
		} else {
			$status = `grep "INCLUDEVISIO" $fname`;
		}
		chomp $status;
		if ( $status ne "" ) {
			UpdateLatexFormat();
			foreach $fentry ( split(/\n/, $status) ) {
				@fields = split(/[{}|]/,$fentry);
				$filename = $fields[1];
				$ofilename = $fields[1];
				$jpg_scale = $fields[2];
				$page_name = $fields[3];
				if ( ! -e $filename ) {
					$filename = ConvertFilename($filename);
					if ( -e $filename ) {
						# filename just needed converting
					} elsif ( -e "${VIEW}${filename}" ) {
						$filename = "$VIEW$filename";
					} elsif ( -e "${VIEW}${DS}${filename}" ) {
						$filename = "$VIEW${DS}$filename";
					} else {
						# If file is still not found, print error.
						print STDERR "ERROR: $filename cannot be located\n";
						exit();
					}
				}
				$jpg_filename = ConvertVisio($filename, $page_name);
				if ( $jpg_filename ne "" ) {
					chdir("${DOCDIR}${DS}html");
					@html_files = <*.html>;
					foreach $hfile ( @html_files ) {
						UpdateHtml( ${hfile}, $ofilename, $jpg_filename, $jpg_scale, $page_name);
					}

					chdir("${DOCDIR}${DS}latex");
					@latex_files = <*.tex>;
					foreach $lfile ( @latex_files ) {
						UpdateLatexFile( ${lfile}, $ofilename, $jpg_filename, $jpg_scale, $page_name);
					}
				}
			}
		}
	}
}

sub Eps2PDF {

	if (!open(FILE, ">eps2pdf.pl")) {
	  print "Error cannot open eps2pdf.pl for output\n";
	  return -1;
	}	

	print FILE <<SCRIPT_END;
#!/usr/bin/perl

foreach \$f (<*.eps>) {
  my \$g = \$f;
  \$g =~ s/.eps/.pdf/;
  if (! -e \$g) {
    my \$status = `epstopdf \$f > \$g`;
  }
}
SCRIPT_END
	close FILE;
	return 0;
}


sub BuildPDF {

	my $PdfConvHost = "srSANDocBuild.pv.com";
	my $PdfConvUser = "docbuild";
	my $PdfConvDir;
	my $timestamp = GenerateDatestr("time");

	print "     >>>>> Preparing $PdfConvHost\n";
	$PdfConvDir = "/home/${PdfConvUser}/latex_${timestamp}";

	$status = `rsh $PdfConvHost -l $PdfConvUser "mkdir -p ${PdfConvDir}" 2>>$LOGFILE`;
	if ( $status =~ /can't establish connection/ ){
		print "Error:  Cannot connect to $PdfConvHost\n";
		return;
	}

	# output the epstopdf script
	if (Eps2PDF()) {
	   print "Error: creating Eps2PDF script\n";
	   return;
	}

	if ($ENV{'OS'} =~ /Windows/) {
		$status = `$RCP * ${PdfConvHost}.${PdfConvUser}:${PdfConvDir}`;
	} else {
		$status = `chmod +w *`;
		$status = `$RCP * ${PdfConvUser}\@${PdfConvHost}:${PdfConvDir} 2>>$LOGFILE`;
	}

	print "     >>>>> Generating PDF.  Please wait....\n";
	$status = `rsh $PdfConvHost -l $PdfConvUser "cd ${PdfConvDir};perl eps2pdf.pl;/opt/rational/clearcase/bin/clearmake" >> $LOGFILE 2>&1`;
	
	if ($ENV{'OS'} =~ /Windows/) {
		$status = `$RCP ${PdfConvHost}.${PdfConvUser}:${PdfConvDir}/* .`;
	} else {
		$status = `$RCP ${PdfConvUser}\@${PdfConvHost}:${PdfConvDir}/* . 2>>$LOGFILE`;
	}

	$status = `rsh $PdfConvHost -l $PdfConvUser "rm -rf ${PdfConvDir}" 2>>$LOGFILE`;
}

sub BuildDocs {

	my $buildtype=$_[0];

	$DOCDIR="${BUILDDIR}${DS}${buildtype}";

	if ( "$INPUTDIR" eq "" ) {
		if ("$buildtype" eq "symbian") {
			$PROJ_NAME="Symbian pvPlayer";
			$INPUTDIR="${VIEW}${DS}sdk${DS}common${DS}enginecommon${DS}base${DS}inc${DS}PVCommonBase.h ${VIEW}${DS}sdk${DS}common${DS}enginecommon${DS}epoc${DS}inc${DS}PVCommonEpoc.h ${VIEW}${DS}sdk${DS}player${DS}engine${DS}base${DS}inc${DS}PVPlayerCommonBase.h ${VIEW}${DS}sdk${DS}player${DS}engine${DS}epoc${DS}inc${DS}PVPlayerCommonEpoc.h ${VIEW}${DS}sdk${DS}player${DS}engine${DS}epoc${DS}inc${DS}PVPlayerInterface.h";
		} else {
			print STDERR "Invalid Document Type:  $buildtype\n";
			usage();
		}
	} else {
		$PROJ_NAME = $buildtype;
	}

	if ( $DOC_TYPE eq "" ) {
		$DOC_TYPE="${PROJ_NAME} API Documentation";
	}

	# Cleanup previous build
	print STDOUT "\n>>>>> Building $DOC_TYPE\n";

	@dir_list = split / /, $INPUTDIR;
	chomp @dir_list;

	$status = `${RMDIR} $DOCDIR >> $LOGFILE 2>&1`;
	$status = `${MKDIR} $DOCDIR`;
	$status = `${RMDIR} $STAGEDIR >> $LOGFILE 2>&1`;
	$status = `${MKDIR} $STAGEDIR`;
	if ($ENV{'OS'} !~ /Windows/) {
		$status = `chmod 777 $DOCDIR`;
		$status = `chmod 777 $STAGEDIR`;
	}
	chdir("${STAGEDIR}");

	$INPUTDIR = "";
	foreach $dir (@dir_list) {
		if ( ! -e $dir ) {
			$dir = ConvertFilename($dir);
			if ( -e $dir ) {
				# dir just needed converting
			} elsif ( -e "${VIEW}${dir}" ) {
				$dir = "$VIEW$dir";
			} elsif ( -e "${VIEW}${DS}${dir}" ) {
				$dir = "$VIEW${DS}$dir";
			} else {
				# If file is still not found, print error.
				print "\n\nERROR: Unable to locate document input directory:\n${dir}\n\n";
				exit();
			}
		}
		$INPUTDIR = "$INPUTDIR $dir";
	}

	# Update footer.html and header.tex
	if ( $AddCopyright ) {
		$status = `${COPY} ${CC_DIR}header_with_copyright.tex ${STAGEDIR}${DS}header.tex`;
	} else {
		$status = `${COPY} ${CC_DIR}header.tex ${STAGEDIR}`;
	}
	$status = `${COPY} ${CC_DIR}footer.html ${STAGEDIR}`;
	$status = `${COPY} ${CC_DIR}doxygen.sty ${STAGEDIR}`;
	$status = `${COPY} ${CC_DIR}$CFGFILE ${STAGEDIR}`;

        ReplaceString ("${STAGEDIR}${DS}footer.html", '%%LABEL%%', '%%LABEL%%', ${BUILDNUM});
        ReplaceString ("${STAGEDIR}${DS}footer.html", '%%DOCTYPE%%', '%%DOCTYPE%%', "${DOC_TYPE}");

        ReplaceString ("${STAGEDIR}${DS}header.tex", '%%LABEL%%', '%%LABEL%%', "${LBUILDNUM}");
        ReplaceString ("${STAGEDIR}${DS}header.tex", '%%DOCTYPE%%', '%%DOCTYPE%%', "${DOC_TYPE}");
        ReplaceString ("${STAGEDIR}${DS}header.tex", '%%DATE%%', '%%DATE%%', "${DATE_STR}");

        ReplaceString (-all, "${STAGEDIR}${DS}doxygen.sty", '%%DOCTYPE%%', '%%DOCTYPE%%', "${DOC_TYPE}");

        ReplaceString ("${STAGEDIR}${DS}${CFGFILE}", '%%PROJECT%%', '%%PROJECT%%', "\"${PROJ_NAME}\"");
        ReplaceString ("${STAGEDIR}${DS}${CFGFILE}", '%%INPUT%%', '%%INPUT%%', "${INPUTDIR}");
        ReplaceString ("${STAGEDIR}${DS}${CFGFILE}", '%%OUTPUT%%', '%%OUTPUT%%', "${DOCDIR}");

	if ( $FILETYPE eq "" ) {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%FILETYPE%%', '%%FILETYPE%%', "*.h");
	} else {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%FILETYPE%%', '%%FILETYPE%%', "$FILETYPE");
	}

	if ( $EXCLUDE eq "" ) {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%EXCLUDE%%', '%%EXCLUDE%%', "");
	} else {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%EXCLUDE%%', '%%EXCLUDE%%', "$EXCLUDE");
	}

	if ( $EXCLUDE_PATTERN eq "" ) {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%EXCLUDE_PATTERN%%', '%%EXCLUDE_PATTERN%%', "");
	} else {
		ReplaceString ("${STAGEDIR}${DS}$CFGFILE", '%%EXCLUDE_PATTERN%%', '%%EXCLUDE_PATTERN%%', "$EXCLUDE_PATTERN");
	}

        if ( $level_string eq ALL ) {
        	ReplaceString ("${STAGEDIR}${DS}${CFGFILE}", '%%SECTIONS%%', '%%SECTIONS%%', "DEBUG RELEASE ALL");
        } else {
        	ReplaceString ("${STAGEDIR}${DS}${CFGFILE}", '%%SECTIONS%%', '%%SECTIONS%%', "$level_string");
        }

	# Build Docs
	if ( -e $DOXYGEN ) {
		$status = `$DOXYGEN $CFGFILE >> $LOGFILE 2>&1`;
		if ($?) {
			print STDERR "Doxygen command failed.  See $LOGFILE for more info.\n";
			exit(-1);
		}
	} else {
		print STDERR "\nERROR:  Unable to locate doxygen tool: \n";
		print STDERR "$DOXYGEN\n";
		exit(-1);
	}

	# Update doxygen.sty
	$status = `${COPY} ${STAGEDIR}${DS}doxygen.sty ${DOCDIR}${DS}latex`;
	$status = `${COPY} ${CC_DIR}copyright.jpg ${DOCDIR}${DS}latex`;
	$status = `${COPY} ${CC_DIR}pvlogo.jpg ${DOCDIR}${DS}latex`;
	$status = `${COPY} ${CC_DIR}pvlogo_small.jpg ${DOCDIR}${DS}latex`;
	$status = `${COPY} ${CC_DIR}pvlogo_small.jpg ${DOCDIR}${DS}html`;

	InstallGraphics();

	print "     >>>>> Creating PDF\n";
	chdir("${DOCDIR}${DS}latex");

	if ($ENV{'OS'} =~ /Windows/) {
		# Check if Latex is installed locally.
		# If it is, use that version.
		# If not, transfer files to PdfConvHost
		if ( -e "${MIKTEX_PATH}\\pdflatex.exe" ) {
			# Setup Path so we find latex files
			$ENV{"PATH"} = "${MIKTEX_PATH};${OldPath}";	
			$status = `clearmake >> $LOGFILE 2>&1`;
		} else {
			BuildPDF();
		}
	} else {
		if ( ( -e "/usr/bin/latex" ) && ( -e "/usr/bin/gs" ) ) {
			$status = `clearmake >> $LOGFILE 2>&1`;
		} else {
			BuildPDF();
		}
	}

	# Check that pdf file now exists.
	if ( ! -s "${DOCDIR}${DS}latex${DS}refman.pdf" ) {
		print STDERR "clearmake command failed.  See $LOGFILE for more info.\n";
		exit(-1);
	} else {
		$status = `${COPY} ${DOCDIR}${DS}latex${DS}refman.pdf ${DOCDIR}${DS}latex${DS}${buildtype}_api.pdf`; 
		$status = `${COPY} ${DOCDIR}${DS}latex${DS}refman.pdf ${DOCDIR}${DS}${buildtype}_api.pdf`; 
	}


}

sub PostWebsite {

	my $buildtype=$_[0];

	if ( -d "/opt/pvserver/web/webapps" ) {
		# Post on web page
		if ( ! -d "/opt/pvserver/web/webapps/docs/${buildtype}" ) {
			$status = `${MKDIR} /opt/pvserver/web/webapps/docs/${buildtype}`;
			if ($ENV{'OS'} !~ /Windows/) {
				$status = `chmod 777 /opt/pvserver/web/webapps/docs/${buildtype}`;
			}
		}
	
		$status = `${COPY} ${DOCDIR}${DS}latex${DS}${buildtype}_api.pdf /opt/pvserver/web/webapps/docs/${buildtype}/${buildtype}_api.pdf`;
	
		chdir("${DOCDIR}${DS}html");
		$status = `${COPY} * /opt/pvserver/web/webapps/docs/${buildtype}`;
	} else {
		print "\n\nThis box does not appear to be setup as a web server.\n";
		print "Documents will not be posted.\n";
		print "Documents are located in ${DOCDIR}\n";
		$PostDocs = 0;
	}
}

sub UpdateIndex {

	# Update front page
	$status = `${COPY} ${CC_DIR}index.html $INDEXFILE`;
        ReplaceString ("${INDEXFILE}", '%%DATE%%', '%%DATE%%', ${DATE_STR});
}

sub Cleanup {

	if ($PostDocs) {
		chdir("${CurrDir}");
		$status = `${RMDIR} $BUILDDIR >> $LOGFILE 2>&1`;
		$status = `${RMDIR} $STAGEDIR >> $LOGFILE 2>&1`;
	}
}

#####################################################
#
#     Environmental Variables
#
#####################################################
$doctype_list_string=undef;
@doc_list = ();
$level_string=undef;
$PostDocs = 0;
$help = undef;

GetOptions('doctype=s'=>\$doctype_list_string, 'title=s'=>\$DOC_TYPE, 'view=s'=>\$VIEW, 'path=s'=>\$INPUTDIR, 'word=s'=>\$WORDLIST, 'filetype=s'=>\$FILETYPE, 'exclude=s'=>\$EXCLUDE, 'exclude_pattern=s'=>\$EXCLUDE_PATTERN, 'level=s'=>\$level_string, 'version=s'=>\$InputVersion, 'post'=>\$PostDocs, 'copyright'=>\$AddCopyright, 'help'=>\$help);

print STDOUT "\n";

if ($help) {
    usage();
}

print STDOUT "Building PacketVideo Documentation\n";

if (defined($WORDLIST)) {
	SetEnv();

	@word_list = split / /, $WORDLIST;
	chomp @word_list;

	foreach $doc (@word_list) {
		$doc = ConvertFilename($doc);
		ConvertWordToPDF($doc);
	}

	exit(0);

} else {
	SetEnv();
	SetCCEnv();
	if (!defined($doctype_list_string)) {
		if (defined($INPUTDIR)) {
			print "\nPath to documents was specified without a document type.\n";
			print "Please specify document type to use: ";
			$doctype_list_string = <STDIN>;
			chomp ($doctype_list_string);
		} else {
			$doctype_list_string="symbian";
		}
	}

	@doc_list = split /,/, $doctype_list_string;
	chomp @doc_list;

	if (!defined($level_string)) {
		$level_string="DEBUG RELEASE ALL";
	}

	foreach $buildtype (@doc_list) {
	
		$buildtype = trim($buildtype);
		BuildDocs ($buildtype);
	
		if ($PostDocs) {
			PostWebsite ($buildtype);
			Cleanup();
		} else {
			print ">>>>> Documents are located in ${DOCDIR}\n";
		}
	}
}

if ($PostDocs) {
	print "\n>>>>> Documents have been posted.\n\n";
	UpdateIndex();
	Cleanup();
}

exit(0);

$ENV{"PATH"} = $OldPath;

__END__

  :endofperl
SET SCRIPT=%0
SET ARG_ONE=%1
SET ARG_TWO=%2
SET ARG_THREE=%3
SET ARG_FOUR=%4
SET ARG_FIVE=%5
SET ARG_SIX=%6
SET ARG_SEVEN=%7
SET ARG_EIGHT=%8
SET ARG_NINE=%9
SHIFT
SHIFT
SHIFT
SHIFT
SHIFT
SHIFT
SHIFT
SHIFT
SHIFT
SET ARG_TEN=%1
SET ARG_ELEVEN=%2
SET ARG_TWELVE=%3
SET ARG_THIRTEEN=%4
SET ARG_FOURTEEN=%5
SET ARG_FIFTEEN=%6
SET ARG_SIXTEEN=%7
SET ARG_SEVENTEEN=%8
SET ARG_EIGHTEEN=%9
  ccperl -e "$s = shift; $c = $s =~ /.bat$/ ? $s : $s.'.bat'; $p = (-x $c) ? '' : '-S '; system('ccperl '.$p.$c.' '.join(' ',@ARGV)); exit $?;" %SCRIPT% %ARG_ONE% %ARG_TWO% %ARG_THREE% %ARG_FOUR% %ARG_FIVE% %ARG_SIX% %ARG_SEVEN% %ARG_EIGHT% %ARG_NINE% %ARG_TEN% %ARG_ELEVEN% %ARG_TWELVE% %ARG_THIRTEEN% %ARG_FOURTEEN% %ARG_FIFTEEN% %ARG_SIXTEEN% %ARG_SEVENTEEN% %ARG_EIGHTEEN%

