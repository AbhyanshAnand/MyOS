; Use the .boot section we defined in the linker script
; *IMPORTANT* The script is ai generated because this is my first project, if anything is wrong here please inform
; *IMPORTANT* There are comments for explanation, these are AI generated to but very explanatory.
section .boot

; --- Multiboot2 Header ---
; This header is crucial for GRUB to load your kernel correctly.
header_start:
    dd 0xe85250d6                ; Magic number for Multiboot2
    dd 0                          ; Architecture (0 for i386/protected mode)
    dd header_end - header_start  ; Total header length
    dd -(0xe85250d6 + 0 + (header_end - header_start)) ; Checksum
    ; End tag (required by Multiboot2 spec)
    dw 0, 0, 8
header_end:

; --- 32-bit Entry Point ---
[bits 32]
global _start
_start:
    ; Set up the stack pointer (ESP for 32-bit, RSP will be set later in 64-bit)
    mov esp, stack_top

    ; 1. Set up Page Tables for Long Mode
    ;    We will identity map the first 1GB of memory (0x0 to 0x40000000).
    ;    This is done using 2MB pages for simplicity.
    ;    PML4 (Page Map Level 4) table: 512 entries, each pointing to a PDPT.
    ;    PDPT (Page Directory Pointer Table): 512 entries, each pointing to a PD.
    ;    PD (Page Directory): 512 entries, each pointing to a 2MB page.

    ; PML4 entry points to PDPT_table
    mov edi, pml4_table
    mov esi, pdpt_table
    mov eax, esi
    or eax, 0x3 ; Present (P=1), Read/Write (RW=1)
    mov [edi], eax
    
    ; PDPT entry points to PD_table
    mov edi, pdpt_table
    mov esi, pd_table
    mov eax, esi
    or eax, 0x3 ; Present (P=1), Read/Write (RW=1)
    mov [edi], eax
    
    ; Fill PD_table with 2MB page entries for the first 1GB
    mov edi, pd_table
    mov ecx, 512 ; 512 entries in the page directory (512 * 2MB = 1GB)
    ; Page flags for 2MB pages:
    ;   0x1   (P=1, Present)
    ;   0x2   (RW=1, Read/Write)
    ;   0x80  (PS=1, Page Size - 2MB page)
    ;   0x8   (PWT=1, Page Write-Through - important for memory-mapped I/O like VGA)
    ;   0x10  (PCD=1, Page-Level Cache Disable - also important for memory-mapped I/O)
    ; Combined: 0x1 | 0x2 | 0x80 | 0x8 | 0x10 = 0x9B
    mov esi, 0x0 | 0x9B ; Initial physical address 0x0, combined flags
.map_2mb_pages:
    mov dword [edi], esi ; Write the page directory entry
    add edi, 8           ; Move to the next entry (each entry is 8 bytes)
    add esi, 0x200000    ; Increment physical address by 2MB for the next page
    loop .map_2mb_pages  ; Loop 512 times

    ; 2. Load PML4 into CR3
    ;    CR3 holds the physical address of the PML4 table.
    mov eax, pml4_table
    mov cr3, eax

    ; 3. Enable PAE (Physical Address Extension)
    ;    Bit 5 of CR4. Required for long mode.
    mov eax, cr4
    or eax, 1 << 5     ; Set PAE bit
    mov cr4, eax

    ; 4. Enable Long Mode (via EFER MSR)
    ;    EFER (Extended Feature Enable Register) MSR is at 0xC0000080.
    ;    Bit 8 (LME - Long Mode Enable) must be set.
    mov ecx, 0xC0000080 ; EFER MSR address
    rdmsr               ; Read EFER into EDX:EAX
    or eax, 1 << 8      ; Set LME bit in EAX
    wrmsr               ; Write EDX:EAX back to EFER

    ; --- Global Descriptor Table (GDT) and Segment Selectors ---
    ; Define the GDT. This GDT is designed for a flat 64-bit memory model.
    ; GDT entries are 8 bytes (64 bits) each.
    ; The 'dq' (define quadword) directive is used for clarity and correctness.

    NULL_SEL    equ 0x00 ; Null descriptor (required, always 0)
    CODE_SEL    equ 0x08 ; Selector for 64-bit Code Segment (offset 8 bytes from gdt_start)
    DATA_SEL    equ 0x10 ; Selector for 64-bit Data Segment (offset 16 bytes from gdt_start)

    align 8 ; Ensure GDT is 8-byte aligned for optimal performance
gdt_start:
    dq 0x0000000000000000 ; 0: Null Descriptor - Required
    
    ; 64-bit Code Segment Descriptor (Flat, Ring 0)
    ; P=1, DPL=0, S=1, Type=1010b (Execute/Read), G=1, L=1 (64-bit), D/B=0
    ; Value: 0x00209A0000000000
    gdt_code: dq 0x00209A0000000000

    ; 64-bit Data Segment Descriptor (Flat, Ring 0)
    ; P=1, DPL=0, S=1, Type=0010b (Read/Write), G=1, L=0, D/B=0 (ignored)
    ; Value: 0x0000920000000000
    gdt_data: dq 0x0000920000000000

gdt_end:

; GDT descriptor structure for LGDT instruction
; Limit is (size of GDT - 1), Base is physical address of GDT
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Limit
    dq gdt_start               ; Base address

    ; Load the new GDT into the GDTR (Global Descriptor Table Register)
    lgdt [gdt_descriptor]

    ; 5. Enable Protected Mode and Paging
    ;    Protected Mode Enable (PE) bit 0 of CR0.
    ;    Paging (PG) bit 31 of CR0.
    ;    Ensure other bits are also set as commonly required (e.g., WP, AM, NE, ET, TS, EM, MP)
    ;    A common robust value for CR0 in 64-bit mode is 0x80000021 (PG | PE | ET)
    mov eax, cr0
    or eax, (1 << 0)   ; Set PE (Protected Mode Enable) bit
    or eax, (1 << 31)  ; Set PG (Paging) bit
    ; Consider also setting other common bits if issues persist, e.g.,
    ; or eax, (1 << 1)  ; MP (Monitor Coprocessor)
    ; or eax, (1 << 2)  ; EM (Emulation)
    ; or eax, (1 << 3)  ; TS (Task Switched)
    ; or eax, (1 << 4)  ; ET (Extension Type)
    ; or eax, (1 << 5)  ; NE (Numeric Error)
    ; or eax, (1 << 18) ; WP (Write Protect)
    mov cr0, eax

    ; Reload segment registers with 64-bit compatible selectors
    ; CS is implicitly loaded by the far jump below.
    ; All other segment registers must be explicitly loaded.
    mov ax, DATA_SEL
    mov ds, ax ; Data Segment
    mov es, ax ; Extra Segment
    mov fs, ax ; FS Segment
    mov gs, ax ; GS Segment
    mov ss, ax ; Stack Segment (crucial for stack operations)

    ; 6. Jump to 64-bit code
    ; This is a far jump (segment:offset) that will correctly switch the CPU
    ; into 64-bit mode by loading the 64-bit CODE_SEL into CS and flushing the pipeline.
    jmp CODE_SEL:long_mode_start

; --- 64-bit Code ---
[bits 64]
long_mode_start:
    ; We are now in 64-bit long mode.

    ; IMPORTANT: Align the stack to 16 bytes before calling C functions.
    ; The x86-64 System V ABI requires RSP to be 16-byte aligned before a CALL.
    ; 'and rsp, 0xFFFFFFFFFFFFFFF0' clears the lower 4 bits, aligning it to a 16-byte boundary.
    and rsp, 0xFFFFFFFFFFFFFFF0

    ; The kernel_main function will be called from here.
    extern kernel_main
    call kernel_main

    ; Halt the CPU indefinitely after kernel_main returns.
    ; This prevents the CPU from executing garbage instructions and crashing.
.hang:
    hlt
    jmp .hang

; --- I/O Port Functions (for C code to use in 64-bit mode) ---
; These functions are correctly placed within the [bits 64] section,
; ensuring they are assembled using 64-bit instruction set.
; Arguments are passed in registers according to the x86-64 System V ABI:
;   - First argument (port) in RDI
;   - Second argument (data) in RSI

; global inb(uint16_t port) - Reads a byte from an I/O port
; The 'port' argument is in RDI. We only need the lower 16 bits for DX.
global inb
inb:
    mov dx, di         ; Move port number from RDI (or DI for 16-bit) to DX
    xor eax, eax       ; Clear EAX register before 'in' instruction
    in al, dx          ; Read byte from port (DX) into AL (lower 8 bits of EAX)
    ret                ; Return (value is in AL/EAX)

; global outb(uint16_t port, uint8_t data) - Writes a byte to an I/O port
; The 'port' argument is in RDI, 'data' argument is in RSI.
global outb
outb:
    mov dx, di         ; Move port number from RDI (or DI for 16-bit) to DX
    mov al, sil        ; Move data byte from SIL (lower 8 bits of RSI) to AL
    out dx, al         ; Write byte from AL to port (DX)
    ret                ; Return

; --- Data and BSS Sections ---
; These sections are for uninitialized data (BSS) and should be page-aligned.
section .bss
align 4096
pml4_table: resb 4096      ; Page Map Level 4 table (4KB)
pdpt_table: resb 4096      ; Page Directory Pointer Table (4KB)
pd_table:   resb 4096      ; Page Directory table (4KB)
; Increased stack size to 32KB (8 pages) for robust operation.
stack_bottom: resb 4096 * 16 
stack_top:
