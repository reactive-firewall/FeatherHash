# Use the official Alpine Linux image as the base image
FROM --platform="linux/${TARGETARCH}" alpine:latest AS base-plus-dash
LABEL version="3.1"
LABEL org.opencontainers.image.title="base-plus-dash"
LABEL org.opencontainers.image.description="Custom Alpine image with the dash shell installed."
LABEL org.opencontainers.image.vendor="individual"
LABEL org.opencontainers.image.licenses="BSD-2-Clause"
SHELL [ "/bin/sh", "-c" ]

RUN --mount=type=cache,target=/var/cache/apk,sharing=locked --network=default \
  apk update \
  && apk add cmd:dash
SHELL [ "/usr/bin/dash", "-l", "-c" ]
ENTRYPOINT ["/usr/bin/dash"]
ENV DASH='/usr/bin/dash'
ENV HOSTNAME="base-plus-dash"
# default to double login
CMD [ "/usr/bin/dash", "-l", "-c", "'exec -a dash /usr/bin/dash -il'" ]

FROM --platform="linux/${TARGETARCH}" base-plus-dash as featherhash-bellows

RUN mkdir -p /FeatherHash/FeatherHash && \
    mkdir -p /FeatherHash/tests

# Install necessary dependencies and build tools
RUN apk update && \
    apk add --no-cache \
    clang \
    llvm \
    cmd:lld

# Copy the project files into the container
COPY build-featherHash.sh /FeatherHash/build-featherHash.sh
COPY FeatherHash/* /FeatherHash/FeatherHash/
COPY LICENSE /FeatherHash/LICENSE

# Set the working directory inside the container
WORKDIR /FeatherHash

# Build the project using the provided build script
RUN chmod +x build-featherHash.sh && \
    dash ./build-featherHash.sh

# Install necessary testing tools
RUN apk update && \
    apk add --no-cache \
    openssl

COPY tests/* /FeatherHash/tests/

# Make the test runner script executable
RUN chmod +x tests/test_*.sh

# Run the tests
RUN dash ./tests/test_runner.sh

# Set the entry point to run the compiled binary
ENTRYPOINT ["./out/bin/sha256sum"]

# Stage 2: Create the final image
# shellcheck disable=SC2154
FROM --platform="linux/${TARGETARCH}" scratch AS featherhash-shasum

# set inherited values
LABEL version="1.1"
LABEL org.opencontainers.image.title="FeatherHash-shasum"
LABEL org.opencontainers.image.description="Empty image with stage0 sha256sum, sha384sum, and sha512sum installed"
LABEL org.opencontainers.image.vendor="individual"
LABEL org.opencontainers.image.licenses="0BSD"

COPY --from=featherhash-bellows /FeatherHash/out/bin /bin
COPY --from=featherhash-bellows /FeatherHash/LICENSE /LICENSE

ENTRYPOINT ["/bin/sha256sum"]
