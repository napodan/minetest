#!/bin/bash
set -e

parallel=`grep -c ^processor /proc/cpuinfo`
host=`head -1 /etc/issue`

dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
libdir=$dir/externals

if [[ "Fedora release 20 (Heisenbug)" == "$host" ]]
then
	toolchain_file=$dir/fedora20/toolchain_mingw64.cmake
else
	toolchain_file=$dir/toolchain_mingw64.cmake
fi
irrlicht_version=1.8.1

# Get minetest_game
cd $dir/../../games
[ -d minetest_game ] && /usr/bin/rm -rf minetest_game
wget https://github.com/minetest/minetest_game/archive/master.zip
unzip master.zip
rm master.zip
mv minetest_game-master minetest_game

# Build the thing
cd $dir/../../
git_hash=`git show | head -c14 | tail -c7`
[ -d _build ] && rm -Rf _build/
mkdir _build
cd _build
cmake .. \
	-DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
	-DCMAKE_INSTALL_PREFIX=/tmp \
	-DVERSION_EXTRA=$git_hash \
	-DBUILD_CLIENT=1 -DBUILD_SERVER=0 \
	\
	-DENABLE_SOUND=1 \
	-DENABLE_CURL=1 \
	-DENABLE_GETTEXT=1 \
	-DENABLE_FREETYPE=1 \
	-DENABLE_LEVELDB=1 \
	\
	-DIRRLICHT_INCLUDE_DIR=$libdir/irrlicht-$irrlicht_version/include \
	-DIRRLICHT_LIBRARY=$libdir/irrlicht-$irrlicht_version/lib/Win64-gcc/libIrrlicht.dll.a \
	-DIRRLICHT_DLL=$libdir/irrlicht-$irrlicht_version/bin/Win64-gcc/Irrlicht.dll \
	\
	-DZLIB_INCLUDE_DIR=$libdir/zlib/include \
	-DZLIB_LIBRARIES=$libdir/zlib/lib/libz.dll.a \
	-DZLIB_DLL=$libdir/zlib/bin/zlib1.dll \
	\
	-DLUA_INCLUDE_DIR=$libdir/luajit/include \
	-DLUA_LIBRARY=$libdir/luajit/libluajit.a \
	\
	-DOGG_INCLUDE_DIR=$libdir/libogg/include \
	-DOGG_LIBRARY=$libdir/libogg/lib/libogg.dll.a \
	-DOGG_DLL=$libdir/libogg/bin/libogg-0.dll \
	\
	-DVORBIS_INCLUDE_DIR=$libdir/libvorbis/include \
	-DVORBIS_LIBRARY=$libdir/libvorbis/lib/libvorbis.dll.a \
	-DVORBIS_DLL=$libdir/libvorbis/bin/libvorbis-0.dll \
	-DVORBISFILE_LIBRARY=$libdir/libvorbis/lib/libvorbisfile.dll.a \
	-DVORBISFILE_DLL=$libdir/libvorbis/bin/libvorbisfile-3.dll \
	\
	-DOPENAL_INCLUDE_DIR=$libdir/openal_stripped/include/AL \
	-DOPENAL_LIBRARY=$libdir/openal_stripped/lib/libOpenAL32.dll.a \
	-DOPENAL_DLL=$libdir/openal_stripped/bin/OpenAL32.dll \
	\
	-DCURL_DLL=$libdir/libcurl/bin/libcurl-4.dll \
	-DCURL_INCLUDE_DIR=$libdir/libcurl/include \
	-DCURL_LIBRARY=$libdir/libcurl/lib/libcurl.dll.a \
	\
	-DFREETYPE_INCLUDE_DIR_freetype2=$libdir/freetype/include/freetype2 \
	-DFREETYPE_INCLUDE_DIR_ft2build=$libdir/freetype/include/freetype2 \
	-DFREETYPE_LIBRARY=$libdir/freetype/lib/libfreetype.dll.a \
	-DFREETYPE_DLL=$libdir/freetype/bin/libfreetype-6.dll \
	\
	-DLEVELDB_INCLUDE_DIR=$libdir/leveldb/include \
	-DLEVELDB_LIBRARY=$libdir/leveldb/lib/libleveldb.dll.a \
	-DLEVELDB_DLL=$libdir/leveldb/bin/libleveldb.dll \
	\
	-DCUSTOM_GETTEXT_PATH=$libdir/gettext \
	-DGETTEXT_MSGFMT=`which msgfmt` \
	-DGETTEXT_DLL=$libdir/gettext/bin/libintl-8.dll \
	-DGETTEXT_ICONV_DLL=$libdir/gettext/bin/libiconv-2.dll \
	-DGETTEXT_INCLUDE_DIR=$libdir/gettext/include \
	-DGETTEXT_LIBRARY=$libdir/gettext/lib/libintl.dll.a

make package -j$parallel

# EOF
