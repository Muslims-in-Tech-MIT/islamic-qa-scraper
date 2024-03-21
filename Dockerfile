# Use the official Ubuntu base image
FROM ubuntu:latest

# Set the working directory
WORKDIR /usr/src/app

# Install g++, make, libcurl, and libxml2 with the development packages
RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libxml2-dev

# Copy the C++ source file and the Makefile into the container
COPY main.cpp .
COPY Makefile .

# Compile the program using the Makefile
RUN make

# Run the application
CMD ["./main"]
