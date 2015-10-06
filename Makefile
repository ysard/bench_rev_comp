build_cpp: 
	cd cpp/build && make clean && cmake .. && make -j 8

run:
	python3 run.py

gen_data:
	echo "Please wait..."
	python3 data/generate_data.py
