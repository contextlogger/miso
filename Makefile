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

all :
	-rm -r build
	sake all cert=self pys60=1
	sake all kits=s60_30,s60_31,s60_32 cert=dev pys60=1
	sake all cert=self pys60=2
	sake all cert=dev pys60=2

release :
	-rm -r build
	sake all release cert=self pys60=1
	sake all release kits=s60_30,s60_31,s60_32 cert=dev pys60=1
	sake all release cert=self pys60=2
	sake all release cert=dev pys60=2

upload_dry :
	sake upload dry=true

upload :
	sake upload

