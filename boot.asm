[org 0x7c00]
bits 16

start:
    mov si, msg

print_loop:
    lodsb
    cmp al, 0
    je done
    mov ah, 0x0E
    int 0x10
    jmp print_loop

done:
    hlt
    jmp $

msg db "Welcome to my simple bootloader!", 0

times 510-($-$$) db 0
dw 0xAA55
