mkdir mytest
mkdir mytest/dir1
mkdir mytest/dir2
mkdir mytest/.hidden_dir

touch mytest/test1.txt
touch mytest/dir1/test1.txt
touch mytest/dir2/test2.txt
touch mytest/.hidden_dir/config
touch mytest/.hidden_file

ln -s test1.txt mytest/symlink_file
ln mytest/test1.txt mytest/hard_link_file
