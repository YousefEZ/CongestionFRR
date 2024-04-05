# Use Ubuntu as the base image FROM ubuntu:latest
FROM ubuntu:latest

# Set the working directory in the container
WORKDIR /usr/workspace

# Install curl and other necessary utilities
RUN apt-get update && \
    apt-get install -y wget && \
    apt-get install -y tar && \
    apt-get install -y python3 python3-pip && \
    apt-get clean

# sys dependencies
RUN apt-get install -y g++ python3 cmake ninja-build git

# recommendations
RUN apt-get install -y ccache gdb valgrind

# python dependencies
RUN apt install -y python3-dev pkg-config python3-setuptools
# RUN python3 -m pip install cppyy==2.4.2

# Download the tar file
ADD https://www.nsnam.org/release/ns-allinone-3.41.tar.bz2 /usr/workspace

# Unpack the tar file
RUN tar xfj /usr/workspace/ns-allinone-3.41.tar.bz2

# Building the application
WORKDIR /usr/workspace/ns-allinone-3.41
RUN ./build.py 

WORKDIR /usr/workspace/ns-allinone-3.41/ns-3.41

# Copy the script into the container
COPY entrypoint.sh /usr/workspace/ns-allinone-3.41/ns-3.41/entrypoint.sh

# Make the script executable
RUN chmod +x /usr/workspace/ns-allinone-3.41/ns-3.41/entrypoint.sh

# Set the script as the entrypoint
ENTRYPOINT ["/usr/workspace/ns-allinone-3.41/ns-3.41/entrypoint.sh"]
