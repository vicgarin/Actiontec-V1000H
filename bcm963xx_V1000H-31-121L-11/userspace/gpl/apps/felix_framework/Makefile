prefix = $(INSTALL_DIR)/usr/local/felix

all: 
	cd Trunk; mvn -Dpackaging=plugins install -DskipTests
	cd Trunk; mvn -Dpackaging=bundle install -DskipTests
	mkdir -p $(prefix)
	cp -rf Trunk/main/bin $(prefix)
	cp -rf Trunk/main/bundle $(prefix)
	cp -rf Trunk/main/conf $(prefix)
clean:
	rm -rf $(prefix)/bin
	rm -rf $(prefix)/bundle
	rm -rf $(prefix)/conf
	cd Trunk; mvn -Dpackaging=plugins uninstall
	cd Trunk; mvn -Dpackaging=bundle uninstall
