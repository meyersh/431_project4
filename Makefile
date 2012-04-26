CXX=g++
#project3: wlm.hpp project3.cpp
#	LANG=C $(CXX) project3.cpp -o project3

neural_trainer: neural_trainer.cpp
	g++ neural_trainer.cpp -lfann -o neural_trainer

fanntest: fanntest.cpp
	g++ fanntest.cpp -lfann -o fanntest

fannrun: fannrun.cpp
	g++ fannrun.cpp -lfann -o fannrun

install: project3.cgi
	cp weights.txt ~/public_html/431/pente/
	cp project3.cgi ~/public_html/431/pente/
	cp -R html/* ~/public_html/431/pente/

clean:
	rm *.core wlm session pente trainer
