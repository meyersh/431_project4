CXX=g++
#project3: wlm.hpp project3.cpp
#	LANG=C $(CXX) project3.cpp -o project3

neural: neural.cpp
	g++ neural.cpp -lfann -o neural

neural_trainer: neural_trainer.cpp
	g++ -O2 neural_trainer.cpp -lfann -o neural_trainer

fanntest: fanntest.cpp
	g++ fanntest.cpp -lfann -o fanntest

fannrun: fannrun.cpp
	g++ fannrun.cpp -lfann -o fannrun

project4.cgi: project4.cgi.cpp
	g++ project4.cgi.cpp -lfann -o project4.cgi

install: project4.cgi
	cp pente.net ~/public_html/431/neuralpente/
	cp session-words.txt ~/public_html/431/neuralpente/
	cp project4.cgi ~/public_html/431/neuralpente/
	cp -R html/* ~/public_html/431/neuralpente/

clean:
	rm *.core wlm session pente trainer
