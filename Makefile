all:
	(mkdir -p release && \
	cd release && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && \
	make && \
	sudo make install)

debug:
	(mkdir -p debug && \
	cd debug && \
	cmake -DCMAKE_BUILD_TYPE=Debug .. && \
	make && \
	sudo make install)

clean:
	(if [ -d "release" ]; then \
	  cd release; make clean; cd ..; \
	fi && \
	if [ -d "debug" ]; then \
	  cd debug; make clean; cd ..; \
	fi && \
	cd test && \
	if [ -d "build" ]; then \
	  cd build; make clean; cd ..; \
	fi && \
	cd ../..)

test:
	(cd test && \
	mkdir -p build && \
	cd build && \
	cmake .. && \
	make)

doc:
	(doxygen doc/Doxyfile)

.PHONY: debug clean test doc
