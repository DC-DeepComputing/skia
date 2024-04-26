.PHONY: local
local:
	echo "Go check out http://localhost:8124/canvaskit/index.html"
	rm -f build
	ln -s ../../modules/canvaskit/build build
	python3 -m http.server 8124
