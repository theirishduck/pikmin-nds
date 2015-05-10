FROM cromo/devkitarm

# --------------------------------------------------------------
# Python 3.3 - Install from Source
# Note: below pulled from python/3.3 and more or less unmodified
# --------------------------------------------------------------

# Install tools we'll use for the python build
RUN apt-get update && \
  apt-get -y install \ 
  build-essential \
  curl \
  git \
  libssl-dev  \
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

# Clone in the dsgx-converter project
RUN mkdir /opt/dsgx-converter \
  && git clone --depth=1 https://github.com/zeta0134/dsgx-converter.git /opt/dsgx-converter \
  && chmod +x /opt/dsgx-converter/model2dsgx.py \
  && ln -s /opt/dsgx-converter/model2dsgx.py /usr/local/bin/model2dsgx
  
CMD make