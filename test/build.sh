#!/etc/profiles/per-user/gunnar/bin/fish
rm -f -- *.txt *.clean
./jplc test.jpl -O3 -s >ref.txt
../compiler test.jpl -O3 >out.txt
python clean.py ref.txt ref.clean
python clean.py out.txt out.clean
rm -f -- *.txt
