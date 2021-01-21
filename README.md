# Betelgeuse
This is definitely an operating system, complete with half a kernel and nothing else!

## Branches
### Master
The Master branch is guaranteed to build and run on my machines. That doesn't mean it will run on anyone else's, or that it will actually run on my machine, but it at least mostly works.

### Dev
The Dev branch is where stuff goes to transfer between computers and to save progress on a feature in the middle of working on it. It might or might not work. It might not even build.

## Building
1. Build a cross compiler - latest gcc, x86_64-elf-gcc. There was something specific I had to do to get the relocations to work but I can't quite remember. I should probably document that at some point.
2. Run `make` in the root directory of the project

## Running
Betelgeuse currently works in QEMU, and might work with Virtualbox with an ISO image depending on whether I have fixed the `make iso` rule.
### QEMU
1. Run `make test`
2. Watch the glory of a few lines of text being printed out if you're lucky.
### VIRTUALBOX
1. Run `make iso`
2. Load the ISO into a Virtualbox VM. It requires EFI turned on.
3. Go to `QEMU` step 2.