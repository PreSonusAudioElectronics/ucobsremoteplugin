#!/bin/bash

set -e

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[ucobscontrolplugin - Error] macOS package script can be run on Darwin-type OS only."
    exit 1
fi

echo "[ucobscontrolplugin] Preparing package build"
export QT_CELLAR_PREFIX="$(/usr/bin/find /usr/local/Cellar/qt -d 1 | sort -t '.' -k 1,1n -k 2,2n -k 3,3n | tail -n 1)"

FILENAME_UNSIGNED="ucobscontrolplugin.pkg"
FILENAME="ucobscontrolplugin.pkg"

echo "[ucobscontrolplugin] Modifying ucobscontrolplugin.so"
install_name_tool \
	-change /tmp/obsdeps/lib/QtWidgets.framework/Versions/5/QtWidgets \
		@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets \
	-change /tmp/obsdeps/lib/QtGui.framework/Versions/5/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui \
	-change /tmp/obsdeps/lib/QtCore.framework/Versions/5/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore \
	-change /tmp/obsdeps/lib/QtNetwork.framework/Versions/5/QtNetwork \
		@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork \
	./build/ucobscontrolplugin.so

# Check if replacement worked
echo "[ucobscontrolplugin] Dependencies for ucobscontrolplugin"
otool -L ./build/ucobscontrolplugin.so

if [[ "$RELEASE_MODE" == "True" ]]; then
	echo "[ucobscontrolplugin] Release mode: Signing plugin binary: ucobscontrolplugin.so"
	codesign --sign "$CODE_SIGNING_IDENTITY" ./build/ucobscontrolplugin.so
else
	echo "[ucobscontrolplugin] Skipped plugin codesigning"
fi

echo "[ucobscontrolplugin] Actual package build"
packagesbuild ./CI/macos/ucobscontrolplugin.pkgproj

if [[ "$RELEASE_MODE" == "True" ]]; then
	echo "[ucobscontrolplugin] Release mode: Renaming ucobscontrolplugin.pkg to $FILENAME_UNSIGNED"
	mv ./release/ucobscontrolplugin.pkg ./release/$FILENAME_UNSIGNED
	
	echo "[ucobscontrolplugin] Signing installer: $FILENAME"
	productsign \
		--sign "$INSTALLER_SIGNING_IDENTITY" \
		./release/$FILENAME_UNSIGNED \
		./release/$FILENAME
	rm ./release/$FILENAME_UNSIGNED

	echo "[ucobscontrolplugin] Submitting installer $FILENAME for notarization"
	zip -r ./release/$FILENAME.zip ./release/$FILENAME
	UPLOAD_RESULT=$(xcrun altool \
		--notarize-app \
		--primary-bundle-id "com.presonus.ucobscontrolplugin" \
		--username "$AC_USERNAME" \
		--password "$AC_PASSWORD" \
		--asc-provider "$AC_PROVIDER_SHORTNAME" \
		--file "./release/$FILENAME.zip")
	rm ./release/$FILENAME.zip

	REQUEST_UUID=$(echo $UPLOAD_RESULT | awk -F ' = ' '/RequestUUID/ {print $2}')
	echo "Request UUID: $REQUEST_UUID"

	echo "[ucobscontrolplugin] Wait for notarization result"
	# Pieces of code borrowed from rednoah/notarized-app
	while sleep 30 && date; do
		CHECK_RESULT=$(xcrun altool \
			--notarization-info "$REQUEST_UUID" \
			--username "$AC_USERNAME" \
			--password "$AC_PASSWORD" \
			--asc-provider "$AC_PROVIDER_SHORTNAME")
		echo $CHECK_RESULT

		if ! grep -q "Status: in progress" <<< "$CHECK_RESULT"; then
			echo "[ucobscontrolplugin] Staple ticket to installer: $FILENAME"
			xcrun stapler staple ./release/$FILENAME
			break
		fi
	done
else
	echo "[ucobscontrolplugin] Skipped installer codesigning and notarization. All done: ../build/release/ucobscontrolplugin.pkg"
fi
