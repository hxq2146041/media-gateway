#mkdir build
#cd build
#cmake -DOPENSSL_INCLUDE_DIRS=/usr/include -DOPENSSL_LIBRARIES="/usr/lib/libssl.a;/usr/lib/libcrypto.a" ..
#apt-get install gtk-doc-tools
#!/bin/bash
NCPU=`cat /proc/cpuinfo | grep -c -i processor`
ROOT_DIR=$(pwd)
#ROOT_PATH=/home/ubuntu/workSpace/janus-gateway/media-gateway
THIRD_PARTY_DIR=${ROOT_DIR}/third_party
INSTALL_PREFIX_DIR=

#aptitude install libmicrohttpd-dev libjansson-dev \
#	libssl-dev libsrtp-dev libsofia-sip-ua-dev libglib2.0-dev \
#	libopus-dev libogg-dev libcurl4-openssl-dev liblua5.3-dev \
#	libconfig-dev pkg-config gengetopt libtool automake

print_usage_and_die() {
  echo "Usage: $0 [-o <output_dir><$(pwd)/install>] [-m <all module><>] build_dir"
  exit 1
}

function build_libevent()
{
	echo "###################start build_libevent###########"
	cd ${THIRD_PARTY_DIR}
	#download libevent
	if [ ! -d "libevent" ];then
		git clone https://github.com/libevent/libevent.git 
		cd libevent && git checkout release-2.1.10-stable
		cd ${THIRD_PARTY_DIR}
	fi
	LIBEVENT_DIR=${THIRD_PARTY_DIR}/libevent
	cd ${LIBEVENT_DIR} \
		&& mkdir build \
		&& cd build \
		&& cmake \
				-DEVENT_LIBRARY_TYPE="SHARED" \
				-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX_DIR} .. \
		&& make -j${NCPU} && sudo make install

	cd ${LIBEVENT_DIR} && rm -fr build
}

function build_boringssl()
{
	echo "###################start build_boringssl###########"
	cd ${THIRD_PARTY_DIR}
	#download boringssl
	if [ ! -d "boringssl" ];then
		git clone https://github.com/google/boringssl.git
		cd boringssl
		sed -i s/" -Werror"//g CMakeLists.txt
	fi
	BORINGSSL_DIR=${THIRD_PARTY_DIR}/boringssl
	cd ${BORINGSSL_DIR} \
		&& mkdir build \
		&& cd build \
		&& cmake -DBUILD_SHARED_LIBS=1 -DCMAKE_CXX_FLAGS="-lrt" .. \
		&& make -j${NCPU} \
		&& cd ${BORINGSSL_DIR} \
		&& cp build/ssl/libssl.so ${INSTALL_PREFIX_DIR}/lib \
		&& cp build/crypto/libcrypto.so ${INSTALL_PREFIX_DIR}/lib \
		&& cp -rf include/openssl ${INSTALL_PREFIX_DIR}/include/openssl

	cd ${BORINGSSL_DIR} && rm -fr build
}

function build_libnice()
{
	echo "###################start build_libnice###########"
	cd ${THIRD_PARTY_DIR}
	#download libnice
	if [ ! -d "libnice" ];then
		git clone https://gitlab.freedesktop.org/libnice/libnice
	fi
	#export PKG_CONFIG_PATH=${INSTALL_PREFIX_DIR}/lib/pkgconfig
	#--with-openssl=${INSTALL_PREFIX_DIR} \
	LIBNICE_DIR=${THIRD_PARTY_DIR}/libnice
	cd ${LIBNICE_DIR}	\
		&& ./autogen.sh \
		&& ./configure \
		--prefix=${INSTALL_PREFIX_DIR} \
		--with-gstreamer=false \
		--enable-gtk-doc=false \
		--with-crypto-library=openssl \
		&& make -j${NCPU} && sudo make install

}

function build_libwebsockets()
{
	echo "###################start build_libwebsockets###########"
	#-DLWS_WITH_BORINGSSL=ON 
	#-DLWS_OPENSSL_LIBRARIES="${INSTALL_PREFIX_DIR}/lib/libssl.so;${INSTALL_PREFIX_DIR}/lib/libcrypto.so" 
	#-DLWS_OPENSSL_INCLUDE_DIRS=${INSTALL_PREFIX_DIR}/include \
	#-DLWS_STATIC_PIC=ON
	cd ${THIRD_PARTY_DIR}
	#download libwebsockets
	if [ ! -d "libwebsockets" ];then
		git clone https://github.com/warmcat/libwebsockets.git
	fi
	LIBWEBSOCKETS_DIR=${THIRD_PARTY_DIR}/libwebsockets
	cd ${LIBWEBSOCKETS_DIR} \
		&& mkdir build \
		&& cd build \
		&& cmake \
			-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX_DIR} \
			-DCMAKE_C_FLAGS="-fPIC" \
			-DLWS_MAX_SMP=1 \
			-DLWS_WITH_STATIC=ON \
			-DLWS_WITH_SHARED=ON \
			-DLWS_STATIC_PIC=ON \
			.. \
		&& make -j${NCPU} && sudo make install
	
		cd ${LIBWEBSOCKETS_DIR} && rm -fr build
}

function build_jsoncpp()
{
	echo "###################start build_jsoncpp###########"
	cd ${THIRD_PARTY_DIR}
	#download jsoncpp
	if [ ! -d "jsoncpp" ];then
		git clone https://github.com/open-source-parsers/jsoncpp.git
	fi
	JSONCPP_DIR=${THIRD_PARTY_DIR}/jsoncpp
	cd ${JSONCPP_DIR} \
		&& mkdir build \
		&& cd build \
		&& cmake \
			-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX_DIR} \
			-DBUILD_SHARED_LIBS=ON \
			-DJSONCPP_WITH_TESTS=OFF \
		  .. \
		&& make -j${NCPU} && sudo make install

	cd ${JSONCPP_DIR} && rm -fr build
}

function build_libconfig()
{
	echo "###################start build_libconfig###########"
	cd ${THIRD_PARTY_DIR}
	#download libconfig
	if [ ! -d "libconfig" ];then
		wget https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.2.tar.gz
		tar xvf libconfig-1.7.2.tar.gz
		mv libconfig-1.7.2 libconfig
		rm -fr libconfig-1.7.2.tar.gz
	fi

	LIB_CONFIG_DIR=${THIRD_PARTY_DIR}/libconfig
	cd ${LIB_CONFIG_DIR} \
		&& ./configure --prefix=${INSTALL_PREFIX_DIR} \
				--enable-shared=true \
				--enable-static=false \
		&& make -j${NCPU} && make install

	cd ${LIB_CONFIG_DIR}
}

function build_glib()
{
	echo "###################start build_glib###########"
	cd ${THIRD_PARTY_DIR}
	#download glib
	if [ ! -d "glib" ];then
		git clone https://github.com/GNOME/glib.git
		cd glib && git checkout 2.64.0
		cd ${THIRD_PARTY_DIR}
	fi
	GLIB_DIR=${THIRD_PARTY_DIR}/glib
	cd ${GLIB_DIR} \
		&& meson \
		  -Dinstalled_tests=false \
		  -Ddtrace=false \
		  -Dfam=false \
		  -Dsystemtap=false \
		  -Dselinux=disabled \
		  -Dgtk_doc=false \
		  -Dman=false \
			-Dinternal_pcre=true \
		  -Dlibmount=disabled \
			-Dprefix=${INSTALL_PREFIX_DIR} \
		  build \
		&& cd build \
		&& ninja && ninja install

	cd ${GLIB_DIR} && rm -fr build	
}


function build_libjwt()
{
	echo "###################start build_libjwt###########"
	cd ${THIRD_PARTY_DIR}
	#download libjwt
	if [ ! -d "glib" ];then
		git clone https://github.com/GNOME/glib.git
		cd glib && git checkout 2.64.0
		cd ${THIRD_PARTY_DIR}
	fi
	GLIB_DIR=${THIRD_PARTY_DIR}/glib
	cd ${GLIB_DIR} \	
}

function build_gateway()
{
	echo "###################start build_gateway###########"
	cd ${ROOT_DIR}
	if [ ! -d "build" ];then
		mkdir build
	fi
	
	#-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX_DIR} 	
	#-DOPENSSL_INCLUDE_DIRS=${INSTALL_PREFIX_DIR}/include 
	#-DOPENSSL_LIBRARIES="${INSTALL_PREFIX_DIR}/lib/libssl.so;${INSTALL_PREFIX_DIR}/lib/libcrypto.so" 
	cd build 	
	cmake \
		.. \
		&& make -j${NCPU}
}


TOKENS=()
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -o|--output-dir)
            INSTALL_PREFIX_DIR="$2"
            shift
            ;;
        -m|--build-module)
            BUILD_MODULE="$2"
            shift
            ;; 
        *)    # unknown option
            TOKENS=("$1")
            ;;
    esac
    shift # past argument
done

if  [ ! -n "$INSTALL_PREFIX_DIR" ] ;then
	echo "INSTALL_PREFIX_DIR not exist using default dir"
	INSTALL_PREFIX_DIR=${ROOT_DIR}/install
fi

if [ ! -d "${INSTALL_PREFIX_DIR}" ];then
	mkdir -p ${INSTALL_PREFIX_DIR} 
fi 


echo ===================================
echo "INSTALL_PREFIX_DIR:${INSTALL_PREFIX_DIR}"
echo ===================================

gengetopt -i gateway.ggo --set-package=gateway --set-version=0.0.2

if [ "${BUILD_MODULE}" = "libevent" ]; then
	build_libevent
elif [ "${BUILD_MODULE}" = "libconfig" ];then
	build_libconfig
elif [ "${BUILD_MODULE}" = "boringssl" ];then
	build_boringssl
elif [ "${BUILD_MODULE}" = "glib" ];then
	build_glib
elif [ "${BUILD_MODULE}" = "libnice" ]; then
	build_libnice
elif [ "${BUILD_MODULE}" = "libwebsockets" ]; then
	build_libwebsockets
elif [ "${BUILD_MODULE}" = "gateway" ]; then
	build_gateway
elif [ "${BUILD_MODULE}" = "jsoncpp" ]; then
	build_jsoncpp
elif [ "${BUILD_MODULE}" = "all" ] || [ "${BUILD_MODULE}" = "a" ]; then 
	#build_libconfig
	build_libevent
	#build_boringssl
	build_libwebsockets
	build_jsoncpp
	#build_glib
	build_libnice	
	build_gateway	
else 
	echo "Error params";
fi

#build_libwebsockets()

cd ${ROOT_DIR}





