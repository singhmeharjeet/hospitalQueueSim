hw6:
	g++ -g -std=c++11 ./main.cpp -o hw6
one:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 0.1 0.1 0.1 0.5 1 1 1 1 847 && make clean
	@echo ""
two:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 311 2.25 2.13 1.6 14 23 31 15 18393 && make clean
	@echo ""
three:
	@echo ""
	@echo "Compiling & Running the main file"
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 1 0.5 0.5 0.5 30 5 2 2 1218 && make clean
	@echo ""
	@echo ""
four:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 0 0.1 0.1 0.5 1 1 1 1 84 && make clean
	@echo ""
five:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 249 2.27 1.0 1.15 16 14 2 23 15573 && make clean
	@echo ""
six:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 100 0.1 0.1 0.5 5 5 5 5 152 && make clean
	@echo ""
seven:
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 ./main.cpp -o hw6 && ./hw6 10 0.01 0.0001 0.5 3 8 3 10 42069 && make clean
	@echo ""
eight:
	@echo ""
	@echo "Compiling & Running the main file"
	@echo ""
	g++ -g -std=c++11 -Wall./main.cpp -o hw6 && ./hw6 10 0.01 0.0001 0.5 3 8 3 10 42069 && make clean
	@echo ""
test:
	./hw6 311 2.25 2.13 1.6 14 23 31 15 22
	@echo ""
	./hw6 311 2.25 2.13 1.6 14 23 31 15 101
	@echo ""
	./hw6 311 2.25 2.13 1.6 14 23 31 15 12
clean:
	@echo ""
	@echo "Cleaning up the directory"
	rm -f core *.o hw6
