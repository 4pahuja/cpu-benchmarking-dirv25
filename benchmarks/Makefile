xlen ?= 64
linesize=$$(($(xlen)/8))
target ?= CUSTOM
ITERATIONS ?= 1
march ?= imafdc
RISCV_PREFIX ?= riscv$(xlen)-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_LINK_OPTS ?= -static -nostartfiles -lgcc -T ./common/link.ld
# for c-class
RISCV_HEX = elf2hex $(linesize) 4194304
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump -D
OUTDIR ?= output
Compiler flags   : -O2 -mabi=lp64d -static -Wl,-Ttext-segment,0x10000  -fgnu89-inline -fno-common -DPERFORMANCE_RUN=1  -lrt
FLAGS_STR = -mcmodel=medany -D$(target) -DPERFORMANCE_RUN=1 -DMAIN_HAS_NOARGC=1 -DHAS_STDIO \
					  -DHAS_PRINTF -DHAS_TIME_H -DUSE_CLOCK -DHAS_FLOAT -DITERATIONS=$(ITERATIONS) \
						-O2  \
						-fgnu89-inline \
						-nostartfiles -nostdlib -fno-builtin-printf -mabi=lp64d -march=rv$(xlen)$(march)

FLAGS = "$(FLAGS_STR)"


.PHONY: dhrystone
dhrystone:
	@echo "Compiling Dhrystone"
	@mkdir -p output/dhrystone
	@$(RISCV_GCC) -I./common -D$(target) -DCONFIG_RISCV64=True -mcmodel=medany -static -std=gnu99\
		-O2 -ffast-math -fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -mabi=lp64d -w -static\
		-nostartfiles -lgcc -c common/crt.S -o output/dhrystone/crt.o
	@$(RISCV_GCC) -I./common -D$(target) -DCONFIG_RISCV64=True -mcmodel=medany -static -std=gnu99\
		-O2 -ffast-math -fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -mabi=lp64d -w -static\
		-nostartfiles -lgcc -c common/syscalls.c -o output/dhrystone/syscalls.o
	$(RISCV_GCC) -I./common -I./dhrystone -DCONFIG_RISCV64=True \
				-DITERATIONS=$(ITERATIONS) -D$(target)=True -mcmodel=medany -static -std=gnu99 -O2 -ffast-math \
				-fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -mabi=lp64d \
				-w -static -nostartfiles -lgcc -T ./common/link.ld -o  $(OUTDIR)/dhrystone/dhry.riscv \
				./dhrystone/dhry_1.c ./dhrystone/dhry_2.c ./output/dhrystone/syscalls.o ./output/dhrystone/crt.o
	@$(RISCV_OBJDUMP)  $(OUTDIR)/dhrystone/dhry.riscv >  $(OUTDIR)/dhrystone/dhry.dump
	@$(RISCV_HEX)  $(OUTDIR)/dhrystone/dhry.riscv 2147483648 >  $(OUTDIR)/dhrystone/code.mem
	@ln -sf $(PWD)/../shakti/* $(OUTDIR)/dhrystone

.PHONY: hello
hello:
	@echo "Compiling Hello-Shakti"
	@mkdir -p output/hello/
	$(RISCV_GCC) -I./common -I./hello -DCONFIG_RISCV64=True \
				-D$(target)=True -mcmodel=medany -static -std=gnu99 -O2 -ffast-math \
				-fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -w -static \
				-nostartfiles -lgcc -T ./common/link.ld -o  $(OUTDIR)/hello/hello.riscv ./hello/hello.c \
				./common/syscalls.c ./common/crt.S
	@$(RISCV_OBJDUMP)  $(OUTDIR)/hello/hello.riscv >  $(OUTDIR)/hello/hello.dump
	@$(RISCV_HEX)  $(OUTDIR)/hello/hello.riscv 2147483648 >  $(OUTDIR)/hello/code.mem
	@ln -sf $(PWD)/../shakti/* $(OUTDIR)/hello/

.PHONY: coremarks
coremarks:
	@echo "Compiling Coremarks"
	@mkdir -p output/coremarks
	@$(RISCV_GCC)  -I./common -D$(target) -DCONFIG_RISCV64=True -mcmodel=medany -static -std=gnu99\
		-O2 -ffast-math -fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -w -static\
		-nostartfiles -lgcc -c common/crt.S -o output/coremarks/crt.o
	@$(RISCV_GCC)  -I./common -D$(target) -DCONFIG_RISCV64=True -mcmodel=medany -static -std=gnu99\
		-O2 -ffast-math -fno-common -fno-builtin-printf -march=rv$(xlen)$(march) -w -static\
		-nostartfiles -lgcc -c common/syscalls.c -o output/coremarks/syscalls.o
	$(RISCV_GCC) -I./common -I./coremarks $(FLAGS_STR) $(RISCV_LINK_OPTS) -o  $(OUTDIR)/coremarks/coremarks.riscv \
		./coremarks/core_util.c ./coremarks/ee_printf.c ./coremarks/core_state.c \
		-DFLAGSTR=\"$(FLAGS)\" \
		./coremarks/core_list_join.c ./coremarks/core_portme.c ./coremarks/core_main.c \
		./coremarks/core_matrix.c ./output/coremarks/crt.o ./output/coremarks/syscalls.o
	@$(RISCV_OBJDUMP)  $(OUTDIR)/coremarks/coremarks.riscv >  $(OUTDIR)/coremarks/coremarks.dump
	@$(RISCV_OBJDUMP) -l -S --source-comment $(OUTDIR)/coremarks/coremarks.riscv >  $(OUTDIR)/coremarks/disasm_help.dump
	@$(RISCV_HEX)  $(OUTDIR)/coremarks/coremarks.riscv 2147483648 >  $(OUTDIR)/coremarks/code.mem
	@ln -sf $(PWD)/../shakti/* $(OUTDIR)/coremarks




.PHONY: clean
clean:
	rm -rf $(OUTDIR)
