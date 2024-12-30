#! /usr/bin/perl
$|=1;

# TODO: Optionally enable -O2 for gcc/llvm based GHDL

use strict;

# Some config
my $bench_dir             = "bench";
my $elabo_dir_multi       = "multi";
my $elabo_dir_one         = "one";
my $elabo_dir_noannot     = "noannot";
my $elabo_dir_strategies  = "strategies";

my $configfile       = "config";
my $elabodata        = "elaboration-data";
my $results_augh     = "results-augh";
my $results_xst_f    = "results-xst-f";
my $results_xst_h    = "results-xst-h";
my $results_ghdl     = "results-ghdl";

my $gnuplot_dir      = "gnuplot";
my $report_full      = "report.pdf";

my $vhdl_dir         = "vhdl";
my $simu_dir         = "simu";

# Possible values:
#   ghdl
#   ghdl-mcode
#   isim (XST suite)
#   xsim (Vivado suite)
#   vsim (Modelsim)
my $simu_profile = "ghdl";

# Select what to do
my $do_clean = 0;
# To launch the campaigns
my $do_elabo_one = 1;
my $do_elabo_one_augh = 1;
my $do_elabo_one_xst_f = 0;
my $do_elabo_one_xst_h = 0;
my $do_elabo_one_tb = 1;
my $do_elabo_one_simu = 1;
my $do_elabo_one_aughvsxst = 0;
my $do_elabo_one_ghdl_allsol = 0;
my $do_elabo_noannot = 0;
my $do_elabo_strategies = 0;
my $do_elabo_multi = 0;
my $do_elabo_multi_augh = 1;
my $do_elabo_multi_xst_f = 0;
my $do_elabo_multi_xst_h = 0;
my $do_elabo_multi_tb = 0;
my $do_elabo_multi_simu = 0;
my $do_graphs = 0;
my $do_graphs_constraint = 0;
my $do_report = 0;

my $errors_nb = 0;

my $multithread = 0;

# Take care: don't generate reports while multithreaded
if($multithread!=0) {
	$do_graphs = 0;
	$do_report = 0;
}



# Where the benchmarks are
my @app_paths = (

	"./idct",
	#"./idct-pp",
	#"./mjpeg",
	#"./simon",
	#"./turbocode",

	#"chstone/adpcm",
	#"chstone/aes",
	#"chstone/dfadd",
	#"chstone/dfdiv",
	#"chstone/dfmul",
	#"chstone/dfsin",
	#"chstone/blowfish",
	#"chstone/gsm",
	#"chstone/jpeg",
	#"chstone/mips",
	#"chstone/motion",
	#"chstone/sha",

);



if($multithread!=0) {

	if(@ARGV==0) {
		# Launch all simulations
		for(my $i=0; $i<@app_paths; $i++) {
			system("./gen.pl '$i' > log.simu$i 2>&1 &");
		}
		print "All simulations launched... they are running in background.\n";
		exit 0;
	}

}

if(@ARGV != 0) {
	# The argument is supposed to be the benchmark path
	@app_paths = ($ARGV[0]);
}


# Target chip for board vc709
my $target_chip    = "xc7vx690t";
my $target_spgrade = "2";
my $target_package = "ffg1761";
my $chip_total_lut = 433200;

# Target chip for board Zybo
if(0) {
	$target_chip    = "xc7z010";
	$target_spgrade = "1";
	$target_package = "clg400";
	$chip_total_lut = 17600;
}

# Target chip for board XUPV5
if(0) {
	$target_chip    = "xc5vlx110t";
	$target_spgrade = "1";
	$target_package = "ff1136";
	$chip_total_lut = 69120;
}

# Some default values
my $target_chip_full = "$target_chip-$target_spgrade-$target_package";
#my $target_chip_full = "$target_chip$target_package-$target_spgrade";
my $default_target_params  = "-chip $target_chip -speed $target_spgrade -pkg $target_package -freq 100M";

# Full FPGA
my $default_augh_parameters = "$default_target_params";
# Specify limit as LUT number
#$default_augh_parameters = "$default_target_params -chip-ratio ".(32000/$chip_total_lut*100)."%";
# For the MJPEG
#$default_augh_parameters = "$default_target_params -chip-ratio ".(60000/$chip_total_lut*100)."%";

my $default_augh_apply_single = 1;
my $default_augh_weight_fork = 0;
my $augh_only_sched = 0;
my $augh_only_elabo = 1;
my $augh_no_dsp = 0;
my $augh_dump_iter_vhdl = 0;
my $augh_dump_estim_drifts = 0;


# Variables used in the process

my $campaign_path = $ENV{"PWD"};
my $app_path;

my $bench_path;
my $app_name;
my $app_file;

# Variables that are parameters for launch_one_full()

my $do_augh;
my $augh_save_file = "";
my $augh_parameters;
my $augh_noannot = 0;
my $augh_apply_single = 1;
my $augh_weight_fork = 0;
# Only used with $augh_save_file to dump final circuit properties
# Used in "multi" run, for the resuts of AUGH and xst
my $augh_target_lut;

my $ghdl_save_file = "";
my $ghdl_iter;

my $do_xst_f = 0;
my $do_xst_h = 0;
my $xst_do_save = 0;
my $xst_is_flat = 1;

my $do_tb;
my $tb_config;

my $do_simu;



if($do_report!=0) {
	system("rm -f ./$report_full");
}

for my $ap (@app_paths) {
	$app_path = $ap;
	if($app_path =~ /^./) { $app_path = $campaign_path.'/'.$app_path; }
	print "Application path: $app_path\n";
	chdir($app_path);

	# Go in the Bench sandbox directory
	if (!-d $bench_dir) {
		print "ERROR : The bench dir $bench_dir was not found.\n";
		next;
	}
	chdir($bench_dir);

	# Clean all bench data if asked
	if($do_clean!=0) {
		#system("rm -rf ./$elabo_dir_multi ./$elabo_dir_one ./$gnuplot_dir");
		#system("rm -rf ./$gnuplot_dir");
		system("rm -rf ./$gnuplot_dir/*");
		#next;
	}

	# Get main benchmark info
	if (!-e $configfile) {
		print "ERROR : The config file $configfile was not found.\n";
		next;
	}
	$app_name = "";
	chomp($app_name = `cat $configfile | head -n 1`);
	$app_file = "";
	chomp($app_file = `cat $configfile | head -n 2 | tail -n 1`);

	if($app_name eq "" || $app_file eq "") {
		print "ERROR : No valid benchmark config was found.\n";
		next;
	}
	print "Found bench name=$app_name file=$app_file\n";

	$app_file = "$app_path/$app_file";

	$tb_config = "";
	chomp($tb_config = `cat $configfile | head -n 3 | tail -n 1`);
	if($tb_config eq "") {
		print "WARNING : No valid testbench config was found.\n";
	}

	# Only push some result files somewhere else
	if(0) {
		my $dest = "~/Data/MyPapers/memoire/resultats/$app_name";

		chdir("$elabo_dir_one");
		system("cp estimation-drifts $dest/");

		next;
	}

	# Launch several synthesis operations, iteratively increment the HW limit
	if($do_elabo_multi!=0) {
		print "Bench '$app_name': Launching MULTI...\n";

		$bench_path = "$app_path/$bench_dir/$elabo_dir_multi";
		system("mkdir -p $bench_path");
		chdir($bench_path);

		# Clean results
		$augh_save_file    = "$bench_path/$results_augh";
		$augh_noannot      = 0;
		$augh_apply_single = $default_augh_apply_single;
		$augh_weight_fork  = $default_augh_weight_fork;

		system("echo '# target_lut augh_iter augh_lut augh_ff augh_comblat augh_cycles augh_gentime' > $augh_save_file");

		$do_xst_f = 0;
		$do_xst_h = 0;

		if($do_elabo_multi_xst_f!=0) {
			$do_xst_f = 1;
			$xst_do_save = 1;
			system("echo '# target_lut xst_lut xst_ff xst_lat xst_gentime' > $results_xst_f");
		}
		if($do_elabo_multi_xst_h!=0) {
			$do_xst_h = 1;
			$xst_do_save = 1;
			system("echo '# target_lut xst_lut xst_ff xst_lat xst_gentime' > $results_xst_h");
		}

		$do_tb = $do_elabo_multi_tb;
		$do_simu = $do_elabo_multi_simu;

		# The actual AUGH calls
		my $percent;
		for($percent=0.5; $percent<=20; $percent*=1.25) {
			$bench_path = "$app_path/$bench_dir/$elabo_dir_multi/percent-$percent";
			system("mkdir -p $bench_path");
			chdir($bench_path);

			# Set config
			$do_augh = $do_elabo_multi_augh;
			$augh_target_lut = $chip_total_lut * $percent / 100.0;
			$augh_parameters = "$default_target_params -chip-ratio $percent%";

			print "Bench '$app_name': Launching with $percent% = $augh_target_lut LUTs...\n";

			# Launch processing
			launch_one_full();

		}  # Iteration on target values
	}

	# Launch one synthesis
	if($do_elabo_strategies!=0) {
		print "Application '$app_name': Launching experiments with all strategies...\n";

		$bench_path = "$app_path/$bench_dir/$elabo_dir_strategies";
		system("mkdir -p $bench_path");
		chdir($bench_path);

		$augh_save_file    = "";
		$augh_parameters   = $default_augh_parameters;
		$augh_noannot      = 0;
		$augh_apply_single = $default_augh_apply_single;
		$augh_weight_fork  = $default_augh_weight_fork;

		$do_xst_f = 0;
		$do_xst_h = 0;

		my $ret;

		# Launch processing

		print "Strategy Group-Estim\n";
		$augh_apply_single = 0;
		$augh_weight_fork = 0;

		$ret = launch_augh();
		#die if($ret!=0);
		system("cp $elabodata ./results-group-estim");

		print "Strategy Single-Estim\n";
		$augh_apply_single = 1;
		$augh_weight_fork = 0;

		$ret = launch_augh();
		#die if($ret!=0);
		system("cp $elabodata ./results-single-estim");

		print "Strategy Group-Precise\n";
		$augh_apply_single = 0;
		$augh_weight_fork = 1;

		$ret = launch_augh();
		#die if($ret!=0);
		system("cp $elabodata ./results-group-precise");

		print "Strategy Single-Precise\n";
		$augh_apply_single = 1;
		$augh_weight_fork = 1;

		$ret = launch_augh();
		#die if($ret!=0);
		system("cp $elabodata ./results-single-precise");
	}

	# Launch one synthesis
	if($do_elabo_one!=0) {
		print "Application '$app_name': Launching one full...\n";

		$bench_path = "$app_path/$bench_dir/$elabo_dir_one";
		system("mkdir -p $bench_path");
		chdir($bench_path);

		# Set config
		$do_augh           = $do_elabo_one_augh;
		$augh_save_file    = "";
		$augh_parameters   = $default_augh_parameters;
		$augh_noannot      = 0;
		$augh_apply_single = $default_augh_apply_single;
		$augh_weight_fork  = $default_augh_weight_fork;

		$do_tb = $do_elabo_one_tb;
		$do_simu = $do_elabo_one_simu;

		$do_xst_f = $do_elabo_one_xst_f;
		$do_xst_h = $do_elabo_one_xst_h;
		$xst_do_save = 0;

		# Launch processing
		launch_one_full();

		if($do_elabo_one_aughvsxst!=0) {
			chdir($bench_path);

			# The result file
			my $f = "augh-vs-xst-one";

			# Optionally erase the file
			#system("echo '' >> $f");

			# AUGH data

			my $l;
			chomp($l = `cat $elabodata | tail -n 1`);
			my @a = split(m/\s+/, $l);

			my $augh_lut = $a[1];
			my $augh_ff = $a[2];
			my $augh_gentime = $a[6];

			chomp($l = `cat log.augh | grep "Maximum latency :" | tail -n 1`);
			$l =~ m/:\s+(\S+)\s*ns$/;
			my $augh_comblat = $1;

			system("echo augh $augh_lut $augh_ff $augh_comblat $augh_gentime > $f");

			# XST data

			my $syrname;
			$syrname = "xstprj/project_flat.syr";
			if(-e $syrname) {
				chomp($l = `cat $syrname | grep "Number of Slice LUTs:"`);
				$l =~ m/:\s+(\S+)\s+/;
				my $xst_lut = $1;
				chomp($l = `cat $syrname | grep "Number of Slice Registers:"`);
				$l =~ m/:\s+(\S+)\s+/;
				my $xst_ff = $1;
				chomp($l = `cat $syrname | grep "Minimum period:"`);
				$l =~ m/period:\s+(\S+)ns\s+/;
				my $xst_comblat = $1;
				chomp($l = `cat $syrname | grep "Total REAL time to Xst completion"`);
				$l =~ m/\s+(\S+)\s+secs\s*$/;
				my $xst_gentime = $1;

				system("echo xst-flat $xst_lut $xst_ff $xst_comblat $xst_gentime >> $f");
			}

			$syrname = "xstprj/project_hier.syr";
			if(-e $syrname) {
				chomp($l = `cat $syrname | grep "Number of Slice LUTs:"`);
				$l =~ m/:\s+(\S+)\s+/;
				my $xst_lut = $1;
				chomp($l = `cat $syrname | grep "Number of Slice Registers:"`);
				$l =~ m/:\s+(\S+)\s+/;
				my $xst_ff = $1;
				chomp($l = `cat $syrname | grep "Minimum period:"`);
				$l =~ m/period:\s+(\S+)ns\s+/;
				my $xst_comblat = $1;
				chomp($l = `cat $syrname | grep "Total REAL time to Xst completion"`);
				$l =~ m/\s+(\S+)\s+secs\s*$/;
				my $xst_gentime = $1;

				system("echo xst-hier $xst_lut $xst_ff $xst_comblat $xst_gentime >> $f");
			}

		}

		if($do_elabo_one_ghdl_allsol!=0) {
			print "Launching GHDL for solution each explored solution...\n";

			$ghdl_save_file = "$bench_path/$results_ghdl";
			system("rm -rf $ghdl_save_file");

			for(my $i=0; ; $i++) {
				$ghdl_iter = $i;

				if($app_name eq "mjpeg") {
					if($i>0) {
						print "INFO : mjpeg app is simulated only for the first solution.\n";
						last;
					}
				}

				chdir($bench_path);

				my $d = "explored-sol-".sprintf("%03u", $i);
				if( !-d($d) ) {
					print "The directory $d was not found. Stopping.\n";
					last;
				}
				print "Launching GHDL for solution $i in folder $d\n";
				chdir($d);

				my $ret;

				$ret = gen_tb("$bench_path/$d/$vhdl_dir");
				if($ret!=0) {
					print "ERROR : testbench generation failed.\n";
					last;
				}

				$ret = launch_ghdl();
				if($ret!=0) {
					print "ERROR : GHDL simulation failed.\n";
					last;
				}

			}  # Iterate on all solution folders

		}

	}

	# Launch one synthesis
	if($do_elabo_noannot!=0) {
		print "Application '$app_name': Launching one without annotations...\n";

		$bench_path = "$app_path/$bench_dir/$elabo_dir_noannot";
		system("mkdir -p $bench_path");
		chdir($bench_path);

		# Set config
		$do_augh = $do_elabo_one_augh;
		$augh_save_file    = "";
		$augh_parameters   = $default_augh_parameters;
		$augh_noannot      = 1;
		$augh_apply_single = $default_augh_apply_single;
		$augh_weight_fork  = $default_augh_weight_fork;

		$do_tb = $do_elabo_one_tb;
		$do_simu = $do_elabo_one_simu;

		$do_xst_f = $do_elabo_one_xst_f;
		$do_xst_h = $do_elabo_one_xst_h;
		$xst_do_save = 0;

		# Launch processing
		launch_one_full();
	}

	# Build the new graphs
	if($do_graphs!=0 || $do_report!=0) {
		my $gnuplot_path = "$app_path/$bench_dir/$gnuplot_dir";
		system("mkdir -p $gnuplot_path");
		chdir($gnuplot_path);

		if($do_graphs!=0) {
			print "Bench '$app_name': Generating the graphs...\n";
			launch_gnuplot();
		}

		# Add a page to the main report
		if($do_report!=0) {
			print "Bench '$app_name': Adding graphs to report...\n";
			launch_report();
			if(-e "$campaign_path/$report_full") {
				system(
					"pdfjoin --outfile $campaign_path/$report_full --rotateoversize false".
					" $campaign_path/$report_full ./report.pdf > log.pdfjoin 2>&1"
				);
			}
			else {
				system("cp ./report.pdf $campaign_path/$report_full");
			}
		}

	}

}  # Loop on all benchmarks

if($do_graphs_constraint!=0) {
	my $gnuplot_path = "$campaign_path/$gnuplot_dir";
	system("mkdir -p $gnuplot_path");
	chdir($gnuplot_path);

	print "Generating the graph about respecting constraints...\n";

	my @list_colors = ("2400ff", "ff0000", "c8008e", "06a72d", "00f8ff", "00cf0b", "ff7d00", "ff2eda");

	my $extension = "svg";
	my $cmd_term = 'set terminal svg size 500,400 dynamic font ",10" linewidth 0.4';

	# The actual Gnuplot call

	open (GNUPLOT, "|-", "gnuplot") or die("can't open gnuplot.\n");

	print GNUPLOT $cmd_term."\n";

	print GNUPLOT '
	set notitle
	set xrange [0:70000]
	set yrange [0:]
	set key left top
	set pointsize 0.2
	set linetype 1 lw 2 lc rgb "black"  pointtype 7
	';

	for(my $i=0; $i<@list_colors; $i++) {
		print GNUPLOT "set linetype ".($i+2)." lw 3 lc rgb 0x".$list_colors[$i]."  pointtype 7\n";
	}

	print GNUPLOT
		'set output "graph-lut-vs-target.'.$extension.'"'."\n".
		'set xlabel "Constraint (number of LUT)"'."\n".
		'set ylabel "Circuit area (number of LUT)"'."\n".
		'plot x with lines lt 1 title "Constraint"';
	;

	for(my $i=0; $i<@app_paths; $i++) {
		$app_path = $app_paths[$i];
		if($app_path =~ /^./) { $app_path = $campaign_path.'/'.$app_path; }

		$app_name = "";
		chomp($app_name = `cat $app_path/$bench_dir/$configfile | head -n 1`);

		my $data_multi  = "$app_path/$bench_dir/$elabo_dir_multi/$results_augh";
		if(-e $data_multi) {
			my $lt = ($i % @list_colors) + 2;
			print GNUPLOT
				', "'.$data_multi.'" using 1:3 with lines lt '.$lt.' title "'.$app_name.'"'.
				', "'.$data_multi.'" using 1:3 with points lt 1 notitle';
			;
		}

	}
	printf GNUPLOT "\n";

	close(GNUPLOT);
}

if($errors_nb > 0) {
	die("Errors found: $errors_nb\n");
}

exit 0;



# Launch one synthesis
sub launch_one_full {
	print "Application '$app_name': Launching in $bench_path...\n";
	chdir($bench_path);

	# Launch AUGH elaboration
	if($do_augh!=0) {
		my $ret = launch_augh();
		if($ret!=0) {
			$errors_nb++;
			print "ERROR : AUGH failed, return value is $ret\n";
			return -1;
		}
	}

	# Launch XST logic synthesis
	if($do_xst_f!=0) {
		$xst_is_flat = 1;
		my $ret = launch_xst();
		if($ret!=0) {
			$errors_nb++;
			print "ERROR : XST failed.\n";
			return -1;
		}
	}
	if($do_xst_h!=0) {
		$xst_is_flat = 0;
		my $ret = launch_xst();
		if($ret!=0) {
			$errors_nb++;
			print "ERROR : XST failed.\n";
			return -1;
		}
	}

	# Generate VHDL testbench
	if($do_tb!=0) {
		my $ret = gen_tb("$bench_path/$vhdl_dir");
		if($ret!=0) {
			$errors_nb++;
			print "ERROR : testbench generation failed.\n";
			return -1;
		}
	}

	# Launch VHDL simulation
	if($do_simu!=0) {
		chdir($bench_path);
		my $ret = launch_simu();
		if($ret!=0) {
			$errors_nb++;
			print "ERROR : VHDL simulation failed.\n";
			return -1;
		}
	}

	return 0;
}

sub launch_augh {
	# Clean previous results and stuff
	system("rm -rf explored-sol-*");
	system("rm -f $elabodata");
	system("rm -rf $vhdl_dir xstprj");

	# Copy benchmark commands for AUGH
	open(FILE, ">", "augh-cmds");

	print FILE '

commands-disp

#target-freq 300M

#build handle-annotations no
#hier auto-asg-propag no
#hier auto-asg-propag-symuse
#hier auto-asg-circular no
#hier auto-inline no
#hier auto-insert-tempregs-inbb
#hier actsimp-verbose
#hier nodesimp-verbose

netlist comps-inline yes
#netlist objective-simu
#netlist objective-human
netlist objective-synth
#netlist fsmretime-lut6
#netlist simp-verbose
#netlist vhd2vl-friendly

#op noshare add sub mul
#op noshare-all
#op auto-mem-readbuf no

#build pingpong-auto no

#map dump-states-en debug-states

load

hier disp-tree -o hier-load.txt

hier upd

hier disp-tree -o hier-simp.txt

#die

#sched use-expensive-compare
#sched use-vexalloc-share
#hier node-loop find line 51 unroll-seq full
';

if($augh_no_dsp!=0) {
	print FILE 'techno cmd use-dsp no'."\n";
}

if($augh_only_sched!=0) {
print FILE '
# Perform only scheduling
#sched depend-dump file sched-depend1.txt
resources-build
sched schedule

#hier disp-tree -o hier-sched1.txt -debug

#hier disp-tree
hier upd
#hier disp-tree

#hier disp-tree -o hier-sched12.txt -debug

# Second time, to check the scheduler and simplification passes
#sched depend-dump file sched-depend2.txt
#resources-build
#sched schedule

#hier disp-tree -o hier-sched2.txt -debug

#hier disp-tree
#hier upd
#hier disp-tree
';
}

if($augh_only_elabo!=0) {
print FILE '
#core fd-skip-wire-seq-auto
#core fd-skip-wire-one-cycle
core fd-skip-unroll-part
#core fd-skip-unroll-full
#core fd-skip-inline-ram
#core fd-skip-inline-rom
#core fd-skip-mem-r
#core fd-skip-addops
#hier en-overbig-loop-unroll

#core debug-show-avail-fd
core debug-show-select-fd
core debug-show-applied-fd

#core elabo-iter-max 20
';

if($augh_noannot!=0) {
	print FILE 'core weigths-without-annotations-en'."\n";
}

if($augh_weight_fork!=0) {
	print FILE 'core fd-weight-exact-en'."\n";
}

if($augh_apply_single!=0) {
	print FILE 'core elabo-fd-max 1'."\n";
}

if($augh_dump_iter_vhdl!=0 || $do_elabo_one_ghdl_allsol!=0) {
	print FILE 'core dump-vhdl-en'."\n";
}

if($augh_dump_estim_drifts!=0) {
	print FILE 'core dump-estim-drifts-en'."\n";
}

print FILE '
# Elaborate the circuit
core graph-file open elaboration-data
#core dupimplem redirect file dupImplem.stdout
core dupimplem redirect none
core elaborate

hier disp-tree -o hier-elabo.txt
';
}

printf FILE '
#map retiming-en

# In case no scheduling nor elaboration was done
hier time upd

#map dump-states-en map-dump-states.txt
#map postmap-timing
postprocess

hier disp-tree -o hier-postproc.txt

#netlist test-retiming

#hier time upd
hier time disp
#hier clockcycles

techno eval

vhdl

# Some commands to replace the VHDL by something special
#sh rm vhdl/*.vhd
#sh cp ../../../zzz-arief-vhdl/vhdl_'.$app_name.'/*.vhd vhdl/

# Force generation of a testbench
#plugin cmd xilinx gen-xst-project
#plugin cmd xilinx gen-vivado-project

backend-gen-prj
#backend-launch

augh-resource
';

	close(FILE);

	gen_augh_makefile();

	# Launch AUGH
	print "Launching AUGH elaboration...\n";
	my $failed = 0;
	my $ret = 0;
	$ret = system("make gen");
	if($ret!=0) {
		print "AUGH failed, return $ret !\n";
		$failed = 1;
	}
	# Parse the AUGH dump to find why it failed
	my $l = "";
	chomp($l = `cat log.augh | grep "The initial design does not respect the resource limits"`);
	if($l ne "") {
		print "Too big, elaboration failed !\n";
		$failed = 1;
	}
	if($failed!=0) {
		return 1;
	}

	# Here the elaboration was successful. Get the details.
	if(length($augh_save_file) > 1) {
		chomp($l = `cat $elabodata | tail -n 1`);
		my @a = split(m/\s+/, $l);

		# Columns:
		# iter, luts, ff, dsp, bram, cycles, realtime, cputime
		my $augh_iter = $a[0];
		my $augh_lut = $a[1];
		my $augh_ff = $a[2];
		my $augh_cycles = $a[5];
		my $augh_time = $a[6];

		my $l;
		chomp($l = `cat log.augh | grep "Maximum latency :" | tail -n 1`);
		$l =~ m/:\s+(\S+)\s*ns$/;
		my $augh_comblat = $1;

		# Col 1  target, lut
		# Col 2  Elabo, nb iter
		# Col 3  Elabo, lut
		# Col 4  Elabo, ff
		# Col 5  Elabo, comblat
		# Col 6  Elabo, cycles
		# Col 7  Elabo, gentime

		system("echo $augh_target_lut $augh_iter $augh_lut $augh_ff $augh_comblat $augh_cycles $augh_time >> $augh_save_file");
	}

	return 0;
}

sub launch_xst {
	system("mkdir -p $bench_path/xstprj");
	chdir("$bench_path/xstprj");

	my $syrname;
	my $xst_save_file;
	if($xst_is_flat!=0) {
		$syrname = "project_flat.syr";
		if($xst_do_save!=0) { $xst_save_file = "../$results_xst_f"; }
		system("sed -e 's/^\\(-keep_hierarchy\\).*\$/\\1 no/' project.xst > sed.tmp");
		system("mv sed.tmp project.xst");
	}
	else {
		$syrname = "project_hier.syr";
		if($xst_do_save!=0) { $xst_save_file = "../$results_xst_h"; }
		system("sed -e 's/^\\(-keep_hierarchy\\).*\$/\\1 yes/' project.xst > sed.tmp");
		system("mv sed.tmp project.xst");
	}

	print "Launching XST synthesis ($syrname)...\n";

	my $ret = system("time make synth > log.xst 2>&1");
	if($ret!=0) {
		print "XST failed code $ret !\n";
		return 1;
	}

	system("cp project.syr $syrname");

	if(length($xst_save_file) > 1) {
		my $l;
		chomp($l = `cat $syrname | grep "Total REAL time to Xst completion"`);
		$l =~ m/\s+(\S+)\s+secs\s*$/;
		my $xst_time = $1;
		chomp($l = `cat $syrname | grep "Number of Slice LUTs:"`);
		$l =~ m/:\s+(\S+)\s+/;
		my $xst_lut = $1;
		chomp($l = `cat $syrname | grep "Number of Slice Registers:"`);
		$l =~ m/:\s+(\S+)\s+/;
		my $xst_ff = $1;
		chomp($l = `cat $syrname | grep "Minimum period:"`);
		$l =~ m/period:\s+(\S+)ns\s+/;
		my $xst_lat = $1;

		# Col 1  target, lut
		# Col 2  XST, lut
		# Col 3  XST, ff
		# Col 4  XST, latency
		# Col 5  XST, gentime

		system("echo $augh_target_lut $xst_lut $xst_ff $xst_lat $xst_time >> $xst_save_file");
	}

	return 0;
}

sub launch_gnuplot {

	my $data_file   = "../$elabo_dir_one/$elabodata";
	my $data_multi  = "../$elabo_dir_multi/$results_augh";
	my $data_xst_f  = "../$elabo_dir_multi/$results_xst_f";
	my $data_xst_h  = "../$elabo_dir_multi/$results_xst_h";

	my $data_noannot  = "../$elabo_dir_noannot/$elabodata";

	my $strategies_se = "../$elabo_dir_strategies/results-single-estim";
	my $strategies_sp = "../$elabo_dir_strategies/results-single-precise";
	my $strategies_ge = "../$elabo_dir_strategies/results-group-estim";
	my $strategies_gp = "../$elabo_dir_strategies/results-group-precise";

	if (!-e $data_file) {
		print "ERROR : The elaboration data file $data_file was not found.\n";
		die;
	}

	my $xst_f_valid = 0;
	my $xst_h_valid = 0;

	if (-e $data_xst_f) {
		my $nblines = `cat $data_xst_f | wc -l`;
		if($nblines>1) { $xst_f_valid = 1; }
	}
	if (-e $data_xst_h) {
		my $nblines = `cat $data_xst_h | wc -l`;
		if($nblines>1) { $xst_h_valid = 1; }
	}

	my $extension = "svg";
	my $cmd_term = 'set terminal svg size 400,250 dynamic font ",10" linewidth 0.4';

	#my $extension = "pdf";
	#my $cmd_term = 'set terminal pdf font ",14"';


	# The actual Gnuplot call

	open(FILE, ">", "gnuplot-cmds");

	print FILE $cmd_term."\n";

	# note : point size 0.3 for little, 0.5 for big

	print FILE '
set notitle
set xrange [0:]
set yrange [0:]
set pointsize 0.5
set linetype 1 lw 3 lc rgb "black"  pointtype 7
set linetype 2 lw 3 lc rgb "red"    pointtype 7
set linetype 3 lw 3 lc rgb "green"  pointtype 7
set linetype 4 lw 3 lc rgb "blue"   pointtype 7
set linetype 5 lw 3 lc rgb 0xa39500 pointtype 7
';

	print FILE '
set output "graph-cycles-vs-lut.'.$extension.'"
set xlabel "Circuit area (number of LUT/FF)"
set ylabel "Circuit latency (clock cycles)"
plot "'.$data_file.'" using 2:6 with linespoint lt 2 title "LUT",\
     "'.$data_file.'" using 3:6 with linespoint lt 3 title "FF"
';

	print FILE '
set output "graph-lut-vs-iter.'.$extension.'"
set xlabel "Iteration"
set ylabel "Circuit area (number of LUT/FF)"
plot "'.$data_file.'" using 1:2 with linespoint lt 2 title "LUT",\
     "'.$data_file.'" using 1:3 with linespoint lt 3 title "FF"
';

	print FILE '
set output "graph-cycles-vs-iter.'.$extension.'"
set xlabel "Iteration"
set ylabel "Circuit latency (clock cycles)"
plot "'.$data_file.'" using 1:6 with linespoint lt 1 notitle
';

	print FILE '
set output "graph-gentime-vs-iter.'.$extension.'"
set xlabel "Iteration"
set ylabel "AUGH generation time (seconds)"
plot "'.$data_file.'" using 1:7 with linespoint lt 1 notitle
';


	if(-e $data_multi) {

		print FILE '
set output "graph-lut-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "Circuit area (number of LUT)"
plot "'.$data_multi.'" using 1:1 with linespoint lt 1 title "Constraint",\
     "'.$data_multi.'" using 1:3 with linespoint lt 2 title "AUGH result"
';

		print FILE '
set output "graph-cycles-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "Circuit latency (clock cycles)"
plot "'.$data_multi.'" using 1:6 with linespoint lt 1 notitle
';

		print FILE '
set output "graph-iter-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "Number of iterations"
plot "'.$data_multi.'" using 1:2 with linespoint lt 1 notitle
';

		print FILE '
set output "graph-gentime-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "AUGH generation time (seconds)"
plot "'.$data_multi.'" using 1:7 with linespoint lt 1 notitle
';

		if($xst_f_valid!=0) {
			print FILE '
set output "graph-AXf-lut-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "Circuit area (number of LUT)"
plot "'.$data_multi.'" using 1:3 with linespoint lt 2 title "AUGH result",\
     "'.$data_xst_f.  '" using 1:2 with linespoint lt 4 title "XST result"
';
			print FILE '
set output "graph-Xf-time-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "XST synthesis time (seconds)"
plot "'.$data_xst_f.'" using 1:3 with linespoint lt 4 notitle
';
		}
		if($xst_h_valid!=0) {
			print FILE '
set output "graph-AXh-lut-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "Circuit area (number of LUT)"
plot "'.$data_multi.'" using 1:3 with linespoint lt 2 title "AUGH result",\
     "'.$data_xst_h.  '" using 1:2 with linespoint lt 4 title "XST result"
';
			print FILE '
set output "graph-Xh-time-vs-target.'.$extension.'"
set xlabel "Constraint (number of LUT)"
set ylabel "XST synthesis time (seconds)"
plot "'.$data_xst_h.'" using 1:3 with linespoint lt 4 notitle
';
		}

	}

	if(-e $data_noannot) {

		print FILE '
set output "graph-normal-noannot.'.$extension.'"
set xlabel "Circuit area (number of LUT)"
set ylabel "Circuit latency (clock cycles)"
plot "'.$data_file.'" using 2:6 with linespoint lt 2 title "With annot.",\
     "'.$data_noannot.'" using 2:6 with linespoint lt 3 title "Without annot."
';

	}

	if(-e $strategies_sp) {

		print FILE '
set terminal svg size 400,350 dynamic font ",10"
#set xrange [0:10500]
set output "graph-strategies.'.$extension.'"
set xlabel "Circuit area (number of LUT)"
set ylabel "Circuit latency (clock cycles)"
plot "'.$strategies_sp.'" using 2:6 with lines  lt 2 title "Single, Precise",\
     "'.$strategies_sp.'" using 2:6 with points lt 1 pointsize 0.3 notitle,\
     "'.$strategies_se.'" using 2:6 with lines  lt 3 title "Single, Estim.",\
     "'.$strategies_se.'" using 2:6 with points lt 1 pointsize 0.3 notitle,\
     "'.$strategies_ge.'" using 2:6 with lines  lt 4 title "Group, Precise",\
     "'.$strategies_ge.'" using 2:6 with points lt 1 pointsize 0.3 notitle,\
     "'.$strategies_gp.'" using 2:6 with lines  lt 5 title "Group, Estim.",\
     "'.$strategies_gp.'" using 2:6 with points lt 1 pointsize 0.3 notitle
';

	}

	close(FILE);

	my $ret = system("gnuplot gnuplot-cmds > log.gnuplot 2>&1");
	if($ret!=0) {
		print "Gnuplot failed code $ret !\n";
		return 1;
	}

	open(FILE, ">", "Makefile");

	print FILE '

ALLSVG := $(wildcard *.svg)
ALLPDF := $(patsubst %.svg, %.pdf, $(ALLSVG))

.PHONY : all

all : $(ALLPDF)

%.pdf: %.svg
	inkscape -z -T -D --export-pdf=$@ --file=$<

';

	close(FILE);

	system("make");

	return 0;
}

sub launch_report {

	my $result_file = "report.tex";

	open (FILE, ">", $result_file) or die("can't open $result_file.\n");

print FILE '
\documentclass[a4paper,onecolumn,11pt]{report}

\usepackage[english]{babel}

% Input character encoding
\usepackage[utf8]{inputenc}

% For other French characters (<<, >>, etc.)
\usepackage[T1]{fontenc}

% To easily insert graphics
\usepackage{graphicx}
% To insert multiple images, possibly using the two columns
\usepackage[caption=false,font=footnotesize]{subfig}

\usepackage[a4paper,top=1.5cm,bottom=1.5cm,left=1.5cm,right=1.5cm]{geometry}
\usepackage[pdftex]{hyperref}


\begin{document}

\pagestyle{empty}  % No page number

\begin{center}
\begin{Large} Results for benchmark '.$app_name.' \end{Large}
\end{center}

%\vspace{2cm}

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-cycles-vs-lut.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-gentime-vs-iter.pdf}
		\end{minipage}
	}
\end{figure}

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{0.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-lut-vs-iter.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{0.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-cycles-vs-iter.pdf}
		\end{minipage}
	}
\end{figure}

';


if (-e "graph-lut-vs-target.pdf") {

print FILE '

\newpage

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-lut-vs-target.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-cycles-vs-target.pdf}
		\end{minipage}
	}
\end{figure}

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{0.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-iter-vs-target.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{0.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-gentime-vs-target.pdf}
		\end{minipage}
	}
\end{figure}

';

}

if (-e "graph-AXf-lut-vs-target.pdf") {

print FILE '

\newpage

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-AXf-lut-vs-target.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-Xf-time-vs-target.pdf}
		\end{minipage}
	}
\end{figure}

';
}

if (-e "graph-AXh-lut-vs-target.pdf") {

print FILE '

\newpage

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-AXh-lut-vs-target.pdf}
		\end{minipage}
	}
	\hfill
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-Xh-time-vs-target.pdf}
		\end{minipage}
	}
\end{figure}

';
}

if (-e "graph-normal-noannot.pdf") {

print FILE '

\newpage

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-normal-noannot.pdf}
		\end{minipage}
	}
\end{figure}

';

}

if (-e "graph-strategies.pdf") {

print FILE '

\begin{figure}[h]
	\subfloat{
		\begin{minipage}[b]{.46\linewidth}
			\centering
			\includegraphics[width=1.1\linewidth]{graph-strategies.pdf}
		\end{minipage}
	}
\end{figure}

';

}

print FILE '
\end{document}
';

	close(FILE);

	system("pdflatex report.tex > log.pdflatex 2>&1");
}

sub gen_augh_makefile {
	open(FILE, ">", "Makefile");

	print FILE '
AUGH_FLAGS = -p xilinx -v '.$augh_parameters.'

SRC = '.$app_file.'

.PHONY: gen gen-dbg tb

# Normal generation

gen:
	time augh $(AUGH_FLAGS) -script augh-cmds $(SRC) > log.augh 2>&1

tb:
	augh $(AUGH_FLAGS) -script augh-tb-cmds $(SRC) > log.augh.tb 2>&1

# For manual debug with gdb

gen-dbg:
	gdb --args augh $(AUGH_FLAGS) -script augh-cmds $(SRC)

tb-dbg:
	gdb --args augh $(AUGH_FLAGS) -script augh-tb-cmds $(SRC)

';

	close(FILE);
}

sub gen_tb {
	my $local_vhdl_dir = shift;

	if($tb_config eq "") {
		print "ERROR : No valid testbench config was found.\n";
		return 1;
	}

	my @tb_params = split(/\s+/, $tb_config);

	my $in_fifo_width = $tb_params[0];
	my $in_vector_file = $tb_params[1];
	my $in_vector_bytes = $tb_params[2];

	my $out_fifo_width = $tb_params[3];
	my $out_vector_file = $tb_params[4];
	my $out_vector_bytes = $tb_params[5];

	my $simu_max_cycles = $tb_params[6];

	print "Testbench IN w=$in_fifo_width F=$in_vector_file B=$in_vector_bytes OUT w=$out_fifo_width F=$out_vector_file B=$out_vector_bytes\n";

	# Copy benchmark commands for AUGH
	open(FILE, ">", "augh-tb-cmds");

	print FILE '

commands-disp

load

hier upd
postprocess

netlist tb-disp-cycles no
netlist tb-err-end-in no
';

	my $str_in = "";
	if($in_vector_file ne "-") {
		$str_in = "-the-in -f $app_path/$in_vector_file -nb $in_vector_bytes";
	}

print FILE '
netlist tb-gen '."-odir $local_vhdl_dir -rawbin -be -cy $simu_max_cycles ".$str_in." ".
"-the-out -stopvectors -f $app_path/$out_vector_file -nb $out_vector_bytes".'

augh-resource
';

	close(FILE);

	gen_augh_makefile();

	# Launch AUGH
	print "Launching testbench generation by AUGH...\n";
	my $ret = 0;
	$ret = system("make tb");
	if($ret!=0) {
		print "AUGH failed, return $ret !\n";
		return -1;
	}
	return 0;
}

sub launch_simu {

	system("mkdir -p ./$simu_dir");
	chdir("$simu_dir");

	# Clean previous results and stuff
	system("rm -rf ./*");

	if($simu_profile eq "ghdl") {
		open(FILE, ">", "Makefile");

		print FILE '
GHDL_FLAGS = --workdir=work --std=02
VHDL_DIR   = ../'.$vhdl_dir.'

.PHONY : build analysis elabo simu simu-wave simu-sdf gtkwave clean

build:
	echo "Analysis..."
	time ghdl -a $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd > log.analysis 2>&1
	#echo "Make..."
	#time ghdl -m $(GHDL_FLAGS) tb > log.make 2>&1
	echo "Elaboration..."
	time ghdl -e $(GHDL_FLAGS) tb > log.elaborate 2>&1

analysis:
	echo "Analysis..."
	ghdl -a $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd

elabo:
	#echo "Make..."
	#ghdl -m $(GHDL_FLAGS) tb
	echo "Elaboration..."
	ghdl -e $(GHDL_FLAGS) tb

simu:
	echo "Running simulation..."
	#./tb
	time ./tb > log.simu 2>&1

simu-wave:
	echo "Running simulation with waveform export..."
	time ./tb --wave=simu.ghw > log.simu 2>&1

simu-sdf:
	echo "Running simulation with delay annotations..."
	time ./tb --sdf=top=$(VHDL_DIR)/top.sdf > log.simu 2>&1

gtkwave:
	echo "Launching GTKWave..."
	#cp ../.gtkwaverc ./
	gtkwave simu.ghw

clean:
	rm -rf work
	mkdir -p work
';

		close(FILE);
	}
	elsif($simu_profile eq "ghdl-mcode") {
		open(FILE, ">", "Makefile");

		print FILE '
GHDL_FLAGS = --workdir=work --std=02
VHDL_DIR   = ../'.$vhdl_dir.'

.PHONY : build analysis elabo simu simu-wave simu-sdf gtkwave clean

build:
	echo "Note: nothing done for target build"

analysis:
	echo "Note: nothing done for target analysis"

elabo:
	echo "Note: nothing done for target elabo"

simu:
	echo "Running simulation..."
	#ghdl -c $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd -r tb
	time ghdl -c $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd -r tb > log.simu 2>&1

simu-wave:
	echo "Running simulation with waveform export..."
	time ghdl -c $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd -r tb --wave=simu.ghw > log.simu 2>&1

simu-sdf:
	echo "Running simulation with delay annotations..."
	time ghdl -c $(GHDL_FLAGS) $(VHDL_DIR)/*.vhd -r tb --sdf=top=$(VHDL_DIR)/top.sdf > log.simu 2>&1

gtkwave:
	echo "Launching GTKWave..."
	#cp ../.gtkwaverc ./
	gtkwave simu.ghw

clean:
	rm -rf work
	mkdir -p work
';

		close(FILE);
	}
	elsif($simu_profile eq "isim") {
		open(FILE, ">", "isim.cmd");

		print FILE '
run all
quit
';

		close(FILE);

		open(FILE, ">", "Makefile");

		print FILE '
VHDL_DIR   = ../'.$vhdl_dir.'

.PHONY : build analysis elabo simu simu-sdf clean

build:
	echo "Analysis..."
	time vhpcomp $(VHDL_DIR)/*.vhd > log.analysis 2>&1
	echo "Elaboration..."
	time fuse tb > log.elabo 2>&1

analysis:
	echo "Analysis..."
	time vhpcomp $(VHDL_DIR)/*.vhd

elabo:
	echo "Elaboration..."
	time fuse tb

simu:
	echo "Running simulation..."
	time ./x.exe -tclbatch isim.cmd > log.simu 2>&1

simu-sdf:
	echo "Running simulation with delay annotations..."
	time vhpcomp $(VHDL_DIR)/top.vhd $(VHDL_DIR)/tb.vhd > log.analysis 2>&1
	echo "Elaboration..."
	time fuse tb --mt off > log.elabo 2>&1
	echo "Running simulation with delay annotations..."
	time ./x.exe -tclbatch isim.cmd -sdfroot /tb/top_i -sdftyp $(VHDL_DIR)/top.sdf > log.simu 2>&1

clean:
	rm -rf isim *.log log.* *.wdb fuse.xmsgs fuseRelaunch.cmd
';

		close(FILE);
	}
	elsif($simu_profile eq "xsim") {
		open(FILE, ">", "Makefile");

		print FILE '
# Note: For documentation, refer to Xilinx ug900 "Vivado Design Suite User Guide - Logic Simulation"

VHDL_DIR   = ../'.$vhdl_dir.'

.PHONY : build analysis elabo simu simu-sdf clean

build:
	echo "Analysis..."
	time xvhdl $(VHDL_DIR)/*.vhd > log.analysis 2>&1
	echo "Elaboration..."
	time xelab tb > log.elabo 2>&1

analysis:
	echo "Analysis..."
	time xvhdl $(VHDL_DIR)/*.vhd

elabo:
	echo "Elaboration..."
	time xelab tb

simu:
	echo "Running simulation..."
	time xsim tb -R > log.simu 2>&1

# Warning: Does not work, actual reason unknown
simu-sdf:
	echo "Updating simulator with delay annotations..."
	time xvhdl $(VHDL_DIR)/top.vhd $(VHDL_DIR)/tb.vhd > log.analysis 2>&1
	time xelab tb --mt off --sdftyp /tb/top_i=$(VHDL_DIR)/top.sdf > log.elabo 2>&1
	$(MAKE) --no-print-directory simu

clean:
	rm -rf *.log log.* *.jou *.pb *.wdb .Xil xsim.dir
';

		close(FILE);
	}
	elsif($simu_profile eq "vsim") {
		open(FILE, ">", "vsim.tcl");

		print FILE '
run -all
quit
';

		close(FILE);

		open(FILE, ">", "Makefile");

		print FILE '
MODELSIM_FLAGS = -2002
VHDL_DIR = ../'.$vhdl_dir.'

.PHONY : build analysis elabo simu clean

build:
	echo "Compilation..."
	vlib work
	time vcom $(MODELSIM_FLAGS) $(VHDL_DIR)/*.vhd > log.analysis 2>&1

analysis:
	echo "Analysis..."
	time vcom $(MODELSIM_FLAGS) $(VHDL_DIR)/*.vhd

elabo:
	echo "Note: Nothing to do for elaboration"

simu:
	echo "Running simulation..."
	time vsim -c tb -do vsim.tcl > log.simu 2>&1

clean:
	rm -rf log.* work
';

		close(FILE);
	}
	else {
		print "ERROR Unknown simulation profile '$simu_profile'\n";
		return -1;
	}

	print "Launching VHDL simulation with profile '$simu_profile'...\n";

	system("make -s clean");

	my $ret = 0;

	$ret = system("make -s build");
	if($ret!=0) {
		print "ERROR VHDL compilation failed!\n";
		return -1;
	}

	$ret = system("make -s simu");
	if($ret!=0) {
		print "ERROR VHDL simulation failed!\n";
		return -1;
	}

	# Get the simulation results

	my $line;

	# Manually search for error messages... by default isim does not return failure
	$line = `cat log.simu | grep -i 'ERROR' | head -n 1`;
	if(length($line) > 0) {
		print "ERROR VHDL simu: Errors were found\n";
		return -1;
	}

	$line = `cat log.simu | grep 'Last output vector read' | head -n 1`;
	if(length($line) == 0) {
		print "ERROR VHDL simu: End not found\n";
		return -1;
	}

	$line =~ /cycle\s+(\d+)/;
	my $counter = $1;
	print "INFO VHDL simu: End at cycle $counter\n";

	if(length($ghdl_save_file) > 1) {
		system("echo $ghdl_iter $counter >> $ghdl_save_file");
	}

	return 0;
}


