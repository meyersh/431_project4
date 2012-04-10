CXX=g++
#project3: wlm.hpp project3.cpp
#	LANG=C $(CXX) project3.cpp -o project3

install: project3.cgi
	cp weights.txt ~/public_html/431/pente/
	cp project3.cgi ~/public_html/431/pente/
	cp -R html/* ~/public_html/431/pente/

clean:
	rm *.core wlm session pente trainer
