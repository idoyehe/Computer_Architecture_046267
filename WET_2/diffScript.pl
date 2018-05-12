#! /usr/bin/perl

system('make');

opendir($DH,'./tests');
while ($file = readdir($DH)){
	if ($file =~ /example/){
		print "input is: $file";
		if ($file =~ /example[1-9]\./){
			system("./bp_main tests/$file  > out1.txt");
		}
		if ($file =~ /example1[0-4]\./){
			system("./bp_main tests/$file  > out1.txt");
		}
		
		$file =~ /(example[0-9]*)/;
		$name = $1;
		@out = `diff out1.txt results/$name\.txt`;
		$numOfRows = @out;
#		$line = shift @out;
		if ($numOfRows == 0){
			print "......[Success]\n";
		} else {
			print "			XXXXXX[Failed!]\n";
		}

	} 	
}
closedir($DH);

unlink('out1.txt');