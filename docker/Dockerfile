FROM cromo/devkitarm

# --------------------------------------------------------------
# Python 3.3 - Install from Source
# Note: below pulled from python/3.3 and more or less unmodified
# --------------------------------------------------------------

# Install tools we'll use for the python build
RUN apt-get update && \
  apt-get -y install \
  build-essential \
  cmake \
  curl \
  git \
  libfreetype6 \
  libssl-dev  \
  libxi6 \
  libxxf86vm1 \
  zlib1g-dev && \
  rm -rf /var/lib/apt/lists/*

# remove several traces of debian python
RUN apt-get purge -y python.*

# http://bugs.python.org/issue19846
# > At the moment, setting "LANG=C" on a Linux system *fundamentally breaks Python 3*, and that's not OK.
ENV LANG C.UTF-8

ENV PYTHON_VERSION 3.3.6

# gpg: key 36580288: public key "Georg Brandl (Python release signing key) <georg@python.org>" imported
RUN gpg --keyserver ha.pool.sks-keyservers.net --recv-keys 26DEA9D4613391EF3E25C9FF0A5B101836580288

RUN set -x \
  && mkdir -p /usr/src/python \
  && curl -SL "https://www.python.org/ftp/python/$PYTHON_VERSION/Python-$PYTHON_VERSION.tar.xz" -o python.tar.xz \
  && curl -SL "https://www.python.org/ftp/python/$PYTHON_VERSION/Python-$PYTHON_VERSION.tar.xz.asc" -o python.tar.xz.asc \
  && gpg --verify python.tar.xz.asc \
  && tar -xJC /usr/src/python --strip-components=1 -f python.tar.xz \
  && rm python.tar.xz* \
  && cd /usr/src/python \
  && ./configure --enable-shared --enable-unicode=ucs4 \
  && make -j$(nproc) \
  && make install \
  && ldconfig \
  && curl -SL 'https://bootstrap.pypa.io/get-pip.py' | python3 \
  && find /usr/local \
    \( -type d -a -name test -o -name tests \) \
    -o \( -type f -a -name '*.pyc' -o -name '*.pyo' \) \
    -exec rm -rf '{}' + \
  && rm -rf /usr/src/python

# make some useful symlinks that are expected to exist
RUN cd /usr/local/bin \
  && ln -s idle3 idle \
  && ln -s pydoc3 pydoc \
  && ln -s python3 python \
  && ln -s python-config3 python-config

# ----------------------------------------------------------------
# End section Python 3.3
# ----------------------------------------------------------------

# Install the libraries that the dsgx-converter relies on
RUN pip install docopt \
  && pip install euclid3 \
  && pip install numpy \
  && pip install Pillow

# Here we download, untar, and install the Autodesk FBX SDK

RUN wget http://images.autodesk.com/adsk/files/fbx20151_fbxsdk_linux.tar.gz \
  && tar zxf fbx20151_fbxsdk_linux.tar.gz \
  && mkdir /opt/FBX_SDK \
  && bash -c "./fbx20151_fbxsdk_linux /opt/FBX_SDK <<< $'yes\nn\n'" \
  && rm fbx20151_fbxsdk_linux.tar.gz Install_FbxSdk.txt fbx20151_fbxsdk_linux

# Now we do the same thing with the python bindings, and install the library
# globally for the dsgx-converter to be able to use

RUN wget http://images.autodesk.com/adsk/files/fbx20151_fbxpythonsdk_linux.tar.gz \
  && tar zxf fbx20151_fbxpythonsdk_linux.tar.gz \
  && mkdir /opt/FBX_SDK/Python \
  && bash -c "./fbx20151_fbxpythonsdk_linux /opt/FBX_SDK/Python <<< $'yes\nn\n'" \
  && cp /opt/FBX_SDK/Python/lib/Python33_x64/* /usr/local/lib/python3.3/site-packages \
  && rm fbx20151_fbxpythonsdk_linux.tar.gz fbx20151_fbxpythonsdk_linux  Install_FbxSdk.txt

# here we install the ASSIMP library, which will maybe replace all of the above
# with sanity. sweet sanity.

RUN cd /home \
  && git clone https://github.com/assimp/assimp.git \
  && cd assimp \
  && cmake -G "Unix Makefiles" \
  && make install \
  && cd port/PyAssimp \
  && python setup.py install \
  && cd /home \
  && rm -rf assimp

# Install Blender, so that it can perform object exports
# directly as part of the build
RUN curl -O http://download.blender.org/release/Blender2.74/blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  tar xf blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  mv blender-2.74-linux-glibc211-x86_64 /opt/ && \
  rm blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  echo '/opt/blender-2.74-linux-glibc211-x86_64/lib' > /etc/ld.so.conf.d/blender.conf && \
  ldconfig

# Patch the FBX exporter for Blender so it doesn't print out
# tons of false positive error messages.
# From https://developer.blender.org/rBA94abe3b97f5c0b1afaa084dc8918464d27689b88
ADD dupli_error.patch /dupli_error.patch
RUN patch -d /opt/blender-2.74-linux-glibc211-x86_64/2.74/scripts/addons/io_scene_fbx < /dupli_error.patch && \
  rm /dupli_error.patch

ENV PATH $PATH:/opt/blender-2.74-linux-glibc211-x86_64

# Clone in the dsgx-converter project
RUN mkdir /opt/dsgx-converter \
  && git clone https://github.com/zeta0134/dsgx-converter.git /opt/dsgx-converter \
  && cd /opt/dsgx-converter \
  && git reset --hard 54dbb5f6d716b55d533e4ea4aa1a37e303cdc94c \
  && chmod +x /opt/dsgx-converter/model2dsgx.py \
  && ln -s /opt/dsgx-converter/model2dsgx.py /usr/local/bin/model2dsgx

# This sets up Hy, a lisp that runs on Python. This is used for the texture
# conversion tool.
RUN pip install hy

CMD make
