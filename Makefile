.PHONY: build-image run-image build-entrena

build-image: 
	docker image build . --tag annclassifier

run-image:
	docker run -v "$(PWD)":/ANNClassifier -it --rm annclassifier /bin/bash

build-entrena: 
	$(MAKE) -C src entrena-ann

