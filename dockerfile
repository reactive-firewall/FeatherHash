# Use the official Alpine Linux image as the base image
FROM --platform="linux/${TARGETARCH}" alpine:latest as featherhash-bellows

RUN mkdir -p /NullForge/Make && \
    mkdir -p /NullForge/tests

# Set the working directory inside the container
WORKDIR /FeatherHash

# Install necessary dependencies and build tools
RUN apk update && \
    apk add --no-cache \
    clang \
    llvm \
    cmd:bash

# Copy the project files into the container
COPY build-featherHash.bash /FeatherHash/build-featherHash.bash
COPY FeatherHash/* /FeatherHash/FeatherHash/
COPY LICENSE /FeatherHash/LICENSE

# Build the project using the provided build script
RUN ./build-featherHash.bash

# Install necessary testing tools
RUN apk update && \
    apk add --no-cache \
    openssl

COPY tests/* /FeatherHash/tests/

# Make the test runner script executable
RUN chmod +x tests/test_*.sh

# Run the tests
RUN ./tests/test_runner.sh

# Set the entry point to run the compiled binary
ENTRYPOINT ["./out/bin/sha256sum"]

# Stage 2: Create the final image
# shellcheck disable=SC2154
FROM --platform="linux/${TARGETARCH}" scratch AS featherhash-shasum

# set inherited values
LABEL version="1.0"
LABEL org.opencontainers.image.title="FeatherHash-shasum"
LABEL org.opencontainers.image.description="Empty image with stage0 sha256sum, sha384sum, and sha512sum installed"
LABEL org.opencontainers.image.vendor="individual"
LABEL org.opencontainers.image.licenses="0BSD"

COPY --from=featherhash-bellows /FeatherHash/out/bin /bin
COPY --from=featherhash-bellows /FeatherHash/LICENSE /LICENSE

ENTRYPOINT ["/bin/sha256sum"]
