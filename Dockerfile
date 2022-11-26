# Base image is Fedora 36
FROM fedora:36

# Do everything in /root instead of /
WORKDIR /root

# Download required files: bzip2, binutils (for strip command), Solana release, BPF tools, and Vote Account Manager
# program source
ADD https://ap.edge.kernel.org/fedora/releases/36/Everything/x86_64/os/Packages/b/bzip2-1.0.8-11.fc36.x86_64.rpm       \
    bzip2-1.0.8-11.fc36.x86_64.rpm
ADD https://ap.edge.kernel.org/fedora/releases/36/Everything/x86_64/os/Packages/b/binutils-2.37-24.fc36.x86_64.rpm     \
    binutils-2.37-24.fc36.x86_64.rpm
ADD https://github.com/solana-labs/solana/releases/download/v1.14.3/solana-release-x86_64-unknown-linux-gnu.tar.bz2    \
    solana-release-1.4.13-x86_64-unknown-linux-gnu.tar.bz2
ADD https://github.com/solana-labs/bpf-tools/releases/download/v1.29/solana-bpf-tools-linux.tar.bz2                    \
    solana-bpf-tools-v1.29-linux.tar.bz2
ADD https://github.com/bji/vote-account-manager/tarball/master                                                         \
    vote-account-manager.tar.gz

# Check SHA-256 hashes to ensure that the expected file contents were downloaded
RUN echo "dfa4e3df8aacf0d5b4243f937cf512cb49cb8d62f27500aa4a7b3414a59c5366"                                            \
         "bzip2-1.0.8-11.fc36.x86_64.rpm" | sha256sum -c -
RUN echo "02f1b4b8d9c471d49abd75952f5af5100fe5229a8a3a293c2c3b699e7ab3b158"                                            \
         "binutils-2.37-24.fc36.x86_64.rpm" | sha256sum -c -
RUN echo "72f1ce7797dce86403e753dca06d92dac5a222b527b1403049d0e7e6cfc066e8"                                            \
         "solana-release-1.4.13-x86_64-unknown-linux-gnu.tar.bz2" | sha256sum -c -
RUN echo "61ab3485168129eb2392efa4e2c7781435c9825f07c19822e46bcf5a2cd8a8d2"                                            \
         "solana-bpf-tools-v1.29-linux.tar.bz2" | sha256sum -c -

# Install bzip2
RUN rpm -i bzip2-1.0.8-11.fc36.x86_64.rpm

# Install binutils.  Dependencies are not needed for /bin/strip.
RUN rpm -i --nodeps binutils-2.37-24.fc36.x86_64.rpm

# Extract just the Solana SDK from the solana release
RUN tar jxf solana-release-1.4.13-x86_64-unknown-linux-gnu.tar.bz2 solana-release/bin/sdk

# Create the expected subdirectory within the Solana SDK for the BPF tools
RUN mkdir -p solana-release/bin/sdk/bpf/dependencies/bpf-tools

# Extract the BPF tools llvm component into the the Solana SDK
RUN (cd solana-release/bin/sdk/bpf/dependencies/bpf-tools; tar jxf /root/solana-bpf-tools-v1.29-linux.tar.bz2 ./llvm)

# Extract just build_program.sh and program
RUN tar zxf vote-account-manager.tar.gz --strip-components=1 "*/build_program.sh" "*/program"

# Cat build_program.sh and contents of all files under program into a single file so that the checksum can be verified
RUN find build_program.sh program -type f | sort | xargs cat > build_contents.txt

# Check that the SHA-256 hash of build_program.sh and program files is as expected
RUN echo "0606f30a4ac3c61880fb4bb291aa94427a6e30f534cbad75f628289d123b1945 build_contents.txt" | sha256sum -c -

# Run build_program.sh to build it
RUN SDK_ROOT=solana-release/bin/sdk SOURCE_ROOT=. sh build_program.sh

# Check to make sure that the sha256sum of the built program is as expected
RUN echo "5cd8d716463defd58fed76cbf711100c753f41202a24c06816873343fd761a0e program.so" | sha256sum -c -
