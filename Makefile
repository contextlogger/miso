default : v1

-include local/custom.mk

v1 :
	sake --trace kits=s60_30 cert=dev pys60=1

v2 :
	sake --trace kits=s60_50 cert=dev pys60=2

v2-gcce4 :
	sake --trace kits=s60_50 cert=dev pys60=2 gcce=4

# This may be useful for building a Miso that installs on
# both 3rd and 5th edition devices without complaints.
5th :
	sake --trace kits=s60_32 cert=self pys60=2 span_5th=true
	sake --trace kits=s60_32 cert=dev pys60=2 span_5th=true

bin-all :
	-rm -r build
	sake all cert=self pys60=1
	sake all kits=s60_30,s60_31,s60_32 cert=dev pys60=1
	sake all cert=self pys60=2
	sake all cert=dev pys60=2

# adds to download/
dist :
	-rm -r build
	sake all release cert=self pys60=1
	sake all release kits=s60_30,s60_31,s60_32 cert=dev pys60=1
	sake all release cert=self pys60=2
	sake all release cert=dev pys60=2

pydoc :
	sake pydoc

MKINDEX := ../tools/bin/make-index-page.rb

.PHONY : web

web :
	-rm -r web
	mkdir web
	cp -a ../tools/web/hiit.css web/
	../tools/bin/txt2tags --target xhtml --infile homepage/index.txt2tags.txt --outfile web/index.html --encoding utf-8 --verbose -C homepage/config.t2t
	cp -a *.txt web/
	cp -a src/miso.py web/
	cp -a homepage/*.html web/
	rsync -av download python-api web/
	$(MKINDEX) web/download
	chmod -R a+rX web
	tidy -utf8 -eq web/older-releases.html

HTDOCS := ../contextlogger.github.com
PAGEPATH := miso
PAGEHOME := $(HTDOCS)/$(PAGEPATH)

release :
	-mkdir -p $(PAGEHOME)
	rsync -av --delete web/ $(PAGEHOME)/

upload :
	cd $(HTDOCS) && git add $(PAGEPATH) && git commit -a -m updates && git push

