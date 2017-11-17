#! /bin/bash

file_gcc=../traces/val_trace_gcc1
file_perl=../traces/val_trace_perl1

obj=./sim_ds

output_dir=../output/
diff_dir=../diff/
val_dir=../validation/

gcc_run() {
    $obj 16 8 1 $file_gcc > $output_dir/gcc1.txt
    $obj 16 8 2 $file_gcc > $output_dir/gcc2.txt
    $obj 60 15 3 $file_gcc > $output_dir/gcc3.txt
    $obj 64 16 8 $file_gcc > $output_dir/gcc4.txt
}

perl_run() {
    $obj 64 16 4 $file_perl > $output_dir/perl5.txt
    $obj 128 16 5 $file_perl > $output_dir/perl6.txt
    $obj 256 64 5 $file_perl > $output_dir/perl7.txt
    $obj 512 64 7 $file_perl > $output_dir/perl8.txt
}

run_diff()
{
    for i in {1..4}
    do
       diff -iw $output_dir/gcc$i.txt $val_dir/val$i.txt > $diff_dir/gcc$i.txt
    done

    for i in {5..8}
    do
       diff -iw $output_dir/perl$i.txt $val_dir/val$i.txt > $diff_dir/perl$i.txt
    done
}

gcc_run
perl_run
run_diff