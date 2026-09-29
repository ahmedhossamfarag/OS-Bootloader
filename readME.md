# Building a 64-bit (X64) UEFI Application using EDK II

This guide provides step-by-step instructions to clone and configure **EDK II**, install the required 64-bit toolchain dependencies on Linux/Ubuntu, compile the EDK II BaseTools, and build a 64-bit UEFI application (`X64`).

---

## Prerequisites & System Requirements

- A Linux environment (Ubuntu / Debian or WSL is recommended).
- Internet access to clone EDK II and its submodules.

---

## Step 1: Install System Dependencies & 64-bit GCC Support

Modern Linux distributions require specific development packages, Python tools, and multi-architecture libraries.

Run the following commands in your terminal:

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    gcc-multilib \
    g++-multilib \
    libc6-dev-i386 \
    uuid-dev \
    iasl \
    git \
    python3-setuptools \
    python3-dev \
    nasm
```

---

## Step 2: Clone and Initialize EDK II

Clone the official Tianocore EDK II repository and initialize its submodules:

```bash
git clone https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init
```

---

## Step 3: Build BaseTools and Configure Environment

EDK II requires its core compilation utilities (`BaseTools`) to be built before you can compile any packages or applications.

1. **Compile BaseTools (both Python scripts and C binaries):**
   ```bash
   make -C BaseTools
   make -C BaseTools/Source/C
   ```

2. **Source the EDK II environment setup script:**
   ```bash
   source edksetup.sh
   ```

---

## Step 5: Copy MyAppPkg to Edk2 Folder

Copy `MyAppPkg` to the Edk2 Folder `/../edk2/`

---

## Step 4: Build Your 64-bit UEFI Application

Once your workspace and environment variables are active, you can compile your custom package for the 64-bit architecture (`X64`) using the generic `GCC` toolchain tag:

```bash
build -p MyAppPkg/MyAppPkg.dsc -a X64 -t GCC
```

### Output Location
After a successful build, your compiled UEFI binary (`.efi`) will be located in the build output directory: `/../edk2/Build/MyApp/DEBUG_GCC/X64`