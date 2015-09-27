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
  python3 \
  python3-pip \
  zlib1g-dev && \
  rm -rf /var/lib/apt/lists/*

# Install the libraries that the dsgx-converter relies on
# This also sets up Hy, a lisp that runs on Python. This is used for the texture
# conversion tool.
RUN pip-3.2 install \
  docopt \
  euclid3 \
  Pillow \
  hy

# Install Blender, so that it can perform object exports
# directly as part of the build
RUN curl -O http://download.blender.org/release/Blender2.74/blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  tar xf blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  mv blender-2.74-linux-glibc211-x86_64 /opt/ && \
  rm blender-2.74-linux-glibc211-x86_64.tar.bz2 && \
  echo '/opt/blender-2.74-linux-glibc211-x86_64/lib' > /etc/ld.so.conf.d/blender.conf && \
  ldconfig

ENV PATH $PATH:/opt/blender-2.74-linux-glibc211-x86_64

# Clone in the dsgx-converter project
RUN mkdir /opt/dsgx-converter \
  && git clone https://github.com/zeta0134/dsgx-converter.git /opt/dsgx-converter \
  && cd /opt/dsgx-converter \
  && git reset --hard c1eff78a52adb635003441bb783bb6a72d7399d8 \
  && chmod +x /opt/dsgx-converter/model2dsgx.py \
  && ln -s /opt/dsgx-converter/model2dsgx.py /usr/local/bin/model2dsgx

CMD make
