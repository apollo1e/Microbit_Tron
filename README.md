# Microbit_Tron - μT-Kernel 3.0 for BBC micro:bit

Real-Time Operating System (RTOS) for BBC micro:bit wireless communication projects.

## Quick Setup

**Install ARM GCC Toolchain:**
- **Ubuntu/Debian:** `sudo apt-get install gcc-arm-none-eabi build-essential`
- **macOS:** `brew install --cask gcc-arm-embedded`
- **Windows:** Download from https://developer.arm.com/downloads/-/gnu-rm and install

## Building Sender/Receiver Applications

### Step 1: Configure Build Output
Edit `mtkernel_3/build_make/makefile`:
```makefile
# Change this line to set output filename:
EXE_FILE := sender    # for sender.hex
# or
EXE_FILE := receiver  # for receiver.hex
```

### Step 2: Configure Application Code
Edit `mtkernel_3/app_sample2/app_main.c`:
- **For Sender:** Uncomment sender code, comment out receiver code
- **For Receiver:** Uncomment receiver code, comment out sender code

### Step 3: Build
```bash
cd mtkernel_3/build_make
make clean
make
# Output: sender.hex or receiver.hex (depending on EXE_FILE setting)
```

### Step 4: Flash to micro:bit
1. Connect micro:bit via USB
2. Copy the `.hex` file to the micro:bit USB drive
3. micro:bit automatically reboots and runs your code

## Building Both Applications

**For Sender:**
1. Set `EXE_FILE := sender` in makefile
2. Configure(comment and uncomment selected and the relevant code blocks ) sender code in `app_main.c`
3. Run `make clean && make`
4. Flash `sender.hex` to first micro:bit

**For Receiver:**
1. Set `EXE_FILE := receiver` in makefile
2. Configure(comment and uncomment selected and the relevant code blocks ) receiver code in `app_main.c`
3. Run `make clean && make`
4. Flash `receiver.hex` to second micro:bit
   ```bash
   # Edit makefile: set EXE_FILE := receiver  
   make clean
   make
   # Output: receiver.hex ready for flashing
   ```

**Windows Notes:**
- Use **Git Bash**, **WSL**, or **MinGW terminal** instead of Command Prompt
- If using WSL, the commands are identical to Ubuntu
- For MinGW, use `mingw32-make` instead of `make`

### Key Build Files

| File | Purpose | When to Edit |
|------|---------|--------------|
| `mtkernel_3/build_make/makefile` | **Main build config** | Change `EXE_FILE`, select `APP` directory |
| `mtkernel_3/app_sample2/app_main.c` | **Your application code** | Write sender/receiver logic here |
| `mtkernel_3/build_make/microbit.mk` | micro:bit compiler settings | Usually don't need to modify |
| `mtkernel_3/app_sample2/subdir.mk` | App-specific build rules | Add new source files here |

## 🔧 Development Workflow

### Creating Sender/Receiver Applications

1. **Edit Application Code:**
   ```bash
   # Your main code goes here:
   nano mtkernel_3/app_sample2/app_main.c
   ```

2. **Configure Build Target:**
   ```bash
   # In mtkernel_3/build_make/makefile, set:
   EXE_FILE := sender    # or receiver
   APP = app_sample2     # your app directory
   ```

3. **Build and Flash:**
   ```bash
   cd mtkernel_3/build_make
   make clean && make
   # Copy *.hex to micro:bit USB drive
   ```

### Building Multiple Applications

The makefile supports switching between different applications:

```makefile
# In makefile, change these variables:
EXE_FILE := sender          # Output filename
APP = app_sample2           # Source directory
TARGET := _MICROBIT_        # Target platform
```

Available sample applications:
- `app_sample2/` - Radio communication examples
- `dly_list1/` - Task delay demonstrations  
- `364_mbit_list/mbf_list3/` - Message buffer examples

## 📁 Important Directories Explained

### `/mtkernel_3/build_make/` - Build System
- **Start here for all builds**
- Contains makefiles for different platforms
- Generated `.hex` files appear here

### `/mtkernel_3/app_sample2/` - Your Application
- `app_main.c` - Write your sender/receiver code here
- `subdir.mk` - Add new source files to this makefile

### `/mtkernel_3/lib/radio/` - Radio Communication
- Custom radio driver for micro:bit 2.4GHz communication
- Provides `radio_init()`, `radio_send()`, `radio_recv()` functions

### `/mtkernel_3/kernel/` - RTOS Core
- **Don't modify** - Contains μT-Kernel 3.0 implementation
- Provides multitasking, synchronization, timers

## 🔌 Flashing to micro:bit

### All Platforms (Drag & Drop Method - Easiest):
1. **Connect micro:bit** via USB (appears as USB drive)
2. **Copy hex file** to micro:bit:
   - **Windows:** Copy from File Explorer to `MICROBIT (D:)` drive
   - **macOS:** Copy to `/Volumes/MICROBIT/`
   - **Linux:** Copy to `/media/MICROBIT/` or `/mnt/MICROBIT/`
3. **micro:bit automatically reboots** and runs your code

### Command Line (Alternative):
```bash
# Linux/macOS:
cp sender.hex /media/MICROBIT/   # or /Volumes/MICROBIT/ on macOS

# Windows (Git Bash/WSL):
cp sender.hex /d/   # if micro:bit appears as D: drive
```

## 🐛 Troubleshooting

### Build Errors
```bash
# Clean and rebuild:
make clean
make

# Check toolchain:
arm-none-eabi-gcc --version

# Verify makefile paths and variables
```

### Common Issues
- **"No rule to make target"** → Check `APP` variable in makefile
- **"arm-none-eabi-gcc not found"** → Install ARM toolchain
- **Radio not working** → Ensure both devices use same channel/group

## 📚 Learning Resources

- **GETTING_STARTED.md** - Detailed setup and examples
- **mtkernel_3/docs/** - μT-Kernel 3.0 documentation
- **Example Applications** - Study `app_sample2/`, `dly_list1/` for patterns

## 🤝 Contributing

1. Fork this repository
2. Create feature branch: `git checkout -b feature/your-feature`
3. Test your changes on actual micro:bit hardware


## 📄 License

This project builds upon μT-Kernel 3.0. See individual source files for license details.

---

